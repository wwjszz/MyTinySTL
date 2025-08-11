#include "variant.h"

#include <gtest/gtest.h>

enum Qual { Ptr, ConstPtr, LRef, ConstLRef, RRef, ConstRRef };

struct get_qual_t {
    constexpr Qual operator()( int* ) const { return Ptr; }
    constexpr Qual operator()( const int* ) const { return ConstPtr; }
    constexpr Qual operator()( int& ) const { return LRef; }
    constexpr Qual operator()( const int& ) const { return ConstLRef; }
    constexpr Qual operator()( int&& ) const { return RRef; }
    constexpr Qual operator()( const int&& ) const { return ConstRRef; }
};

constexpr get_qual_t get_qual{};

struct CopyConstruction : std::exception {};
struct CopyAssignment : std::exception {};
struct MoveConstruction : std::exception {};
struct MoveAssignment : std::exception {};

struct copy_thrower_t {
    constexpr copy_thrower_t() {}
    [[noreturn]] copy_thrower_t( const copy_thrower_t& ) { throw CopyConstruction{}; }
    copy_thrower_t( copy_thrower_t&& ) = default;
    copy_thrower_t& operator=( const copy_thrower_t& ) { throw CopyAssignment{}; }
    copy_thrower_t& operator=( copy_thrower_t&& ) = default;
};

inline bool operator<( const copy_thrower_t&, const copy_thrower_t& ) noexcept { return false; }

inline bool operator>( const copy_thrower_t&, const copy_thrower_t& ) noexcept { return false; }

inline bool operator<=( const copy_thrower_t&, const copy_thrower_t& ) noexcept { return true; }

inline bool operator>=( const copy_thrower_t&, const copy_thrower_t& ) noexcept { return true; }

inline bool operator==( const copy_thrower_t&, const copy_thrower_t& ) noexcept { return true; }

inline bool operator!=( const copy_thrower_t&, const copy_thrower_t& ) noexcept { return false; }

struct move_thrower_t {
    constexpr move_thrower_t() {}
    move_thrower_t( const move_thrower_t& ) = default;
    [[noreturn]] move_thrower_t( move_thrower_t&& ) { throw MoveConstruction{}; }
    move_thrower_t& operator=( const move_thrower_t& ) = default;
    move_thrower_t& operator=( move_thrower_t&& ) { throw MoveAssignment{}; }
};

inline bool operator<( const move_thrower_t&, const move_thrower_t& ) noexcept { return false; }

inline bool operator>( const move_thrower_t&, const move_thrower_t& ) noexcept { return false; }

inline bool operator<=( const move_thrower_t&, const move_thrower_t& ) noexcept { return true; }

inline bool operator>=( const move_thrower_t&, const move_thrower_t& ) noexcept { return true; }

inline bool operator==( const move_thrower_t&, const move_thrower_t& ) noexcept { return true; }

inline bool operator!=( const move_thrower_t&, const move_thrower_t& ) noexcept { return false; }

TEST( Assign_Copy, SameType ) {
    struct Obj {
        constexpr Obj() {}
        Obj( const Obj& ) noexcept { EXPECT_TRUE( false ); }
        Obj( Obj&& ) = default;
        Obj& operator=( const Obj& ) noexcept {
            EXPECT_TRUE( true );
            return *this;
        }
        Obj& operator=( Obj&& ) = delete;
    };
    // `v`, `w`.
    wyne::variant<Obj, int> v, w;
    // copy assignment.
    v = w;
}

TEST( Assign_Copy, DiffType ) {
    struct Obj {
        constexpr Obj() {}
        Obj( const Obj& ) noexcept { EXPECT_TRUE( true ); }
        Obj( Obj&& ) = default;
        Obj& operator=( const Obj& ) noexcept {
            EXPECT_TRUE( false );
            return *this;
        }
        Obj& operator=( Obj&& ) = delete;
    };
    // `v`, `w`.
    wyne::variant<Obj, int> v( 42 ), w;
    // copy assignment.
    v = w;
}

TEST( Assign_Copy, ValuelessByException ) {
    wyne::variant<int, move_thrower_t> v( 42 );
    // std::cout << v.index() << std::endl;
    // std::cout << static_cast<std::size_t>( -1 ) << std::endl;
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    // v = move_thrower_t{};
    // std::cout << v.index() << std::endl;
    EXPECT_TRUE( v.valueless_by_exception() );
    wyne::variant<int, move_thrower_t> w( 42 );
    w = v;
    EXPECT_TRUE( w.valueless_by_exception() );
}

TEST( Assign_Fwd, SameType ) {
    wyne::variant<int, std::string> v( 101 );
    EXPECT_EQ( 101, wyne::get<int>( v ) );
    v = 202;
    EXPECT_EQ( 202, wyne::get<int>( v ) );
}

TEST( Assign_Fwd, DiffType ) {
    wyne::variant<int, std::string> v( 42 );
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    v = "42";
    EXPECT_EQ( "42", wyne::get<std::string>( v ) );
}

TEST( Assign_Fwd, ExactMatch ) {
    wyne::variant<const char*, std::string> v;
    v = std::string( "hello" );
    EXPECT_EQ( "hello", wyne::get<std::string>( v ) );
}

TEST( Assign_Fwd, BetterMatch ) {
    wyne::variant<int, double> v;
    // `char` -> `int` is better than `char` -> `double`
    v = 'x';
    EXPECT_EQ( static_cast<int>( 'x' ), wyne::get<int>( v ) );
}

TEST( Assign_Fwd, NoMatch ) {
    struct x {};
    static_assert( !std::is_assignable<wyne::variant<int, std::string>, x>{}, "variant<int, std::string> v; v = x;" );
}

TEST( Assign_Fwd, WideningOrAmbiguous ) {
#if defined( __clang__ ) || !defined( __GNUC__ ) || __GNUC__ >= 5
    static_assert( std::is_assignable<wyne::variant<short, long>, int>{}, "variant<short, long> v; v = 42;" );
#else
    static_assert( !std::is_assignable<wyne::variant<short, long>, int>{}, "variant<short, long> v; v = 42;" );
#endif
}

TEST( Assign_Fwd, SameTypeOptimization ) {
    wyne::variant<int, std::string> v( "hello world!" );
    // Check `v`.
    const std::string& x = wyne::get<std::string>( v );
    EXPECT_EQ( "hello world!", x );
    // Save the "hello world!"'s capacity.
    auto capacity = x.capacity();
    // Use `std::string::operator=(const char *)` to assign into `v`.
    v = "hello";
    // Check `v`.
    const std::string& y = wyne::get<std::string>( v );
    EXPECT_EQ( "hello", y );
    // Since "hello" is shorter than "hello world!", we should have preserved the
    // existing capacity of the string!.
    EXPECT_EQ( capacity, y.capacity() );
}

TEST( Assign_Fwd, ThrowOnAssignment ) {
    wyne::variant<int, move_thrower_t> v( wyne::in_place_type_t<move_thrower_t>{} );
    // Since `variant` is already in `move_thrower_t`, assignment optimization
    // kicks and we simply invoke
    // `move_thrower_t &operator=(move_thrower_t &&);` which throws.
    EXPECT_THROW( v = move_thrower_t{}, MoveAssignment );
    EXPECT_FALSE( v.valueless_by_exception() );
    EXPECT_EQ( 1u, v.index() );
    // We can still assign into a variant in an invalid state.
    v = 42;
    // Check `v`.
    EXPECT_FALSE( v.valueless_by_exception() );
    EXPECT_EQ( 42, wyne::get<int>( v ) );
}

#if 0
TEST(Assign_Fwd, ThrowOnTemporaryConstruction) {
  wyne::variant<int, copy_thrower_t> v(42);
  // Since `copy_thrower_t`'s copy constructor always throws, we will fail to
  // construct the variant. This results in our variant staying in
  // its original state.
  copy_thrower_t copy_thrower{};
  EXPECT_THROW(v = copy_thrower, CopyConstruction);
  EXPECT_FALSE(v.valueless_by_exception());
  EXPECT_EQ(0u, v.index());
  EXPECT_EQ(42, wyne::get<int>(v));
}

TEST(Assign_Fwd, ThrowOnVariantConstruction) {
  wyne::variant<int, move_thrower_t> v(42);
  // Since `move_thrower_t`'s copy constructor never throws, we successfully
  // construct the temporary object by copying `move_thrower_t`. We then
  // proceed to move the temporary object into our variant, at which point
  // `move_thrower_t`'s move constructor throws. This results in our `variant`
  // transitioning into the invalid state.
  move_thrower_t move_thrower;
  EXPECT_THROW(v = move_thrower, MoveConstruction);
  EXPECT_TRUE(v.valueless_by_exception());
  // We can still assign into a variant in an invalid state.
  v = 42;
  // Check `v`.
  EXPECT_FALSE(v.valueless_by_exception());
  EXPECT_EQ(42, wyne::get<int>(v));
}
#endif

TEST( Assign_Move, SameType ) {
    struct Obj {
        constexpr Obj() {}
        Obj( const Obj& ) = delete;
        Obj( Obj&& ) noexcept { EXPECT_TRUE( false ); }
        Obj& operator=( const Obj& ) = delete;
        Obj& operator=( Obj&& ) noexcept {
            EXPECT_TRUE( true );
            return *this;
        }
    };
    // `v`, `w`.
    wyne::variant<Obj, int> v, w;
    // move assignment.
    v = wyne::move( w );
}

TEST( Assign_Move, DiffType ) {
    struct Obj {
        constexpr Obj() {}
        Obj( const Obj& ) = delete;
        Obj( Obj&& ) noexcept { EXPECT_TRUE( true ); }
        Obj& operator=( const Obj& ) = delete;
        Obj& operator=( Obj&& ) noexcept {
            EXPECT_TRUE( false );
            return *this;
        }
    };
    // `v`, `w`.
    wyne::variant<Obj, int> v( 42 ), w;
    // move assignment.
    v = wyne::move( w );
}

TEST( Assign_Move, ValuelessByException ) {
    wyne::variant<int, move_thrower_t> v( 42 );
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    EXPECT_TRUE( v.valueless_by_exception() );
    wyne::variant<int, move_thrower_t> w( 42 );
    w = wyne::move( v );
    EXPECT_TRUE( w.valueless_by_exception() );
}

TEST( Ctor_Copy, Value ) {
    // `v`
    wyne::variant<int, std::string> v( "hello" );
    EXPECT_EQ( "hello", wyne::get<std::string>( v ) );
    // `w`
    wyne::variant<int, std::string> w( v );
    EXPECT_EQ( "hello", wyne::get<std::string>( w ) );
    // Check `v`
    EXPECT_EQ( "hello", wyne::get<std::string>( v ) );

    /* constexpr */ {
        // `cv`
        constexpr wyne::variant<int, const char*> cv( 42 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
        // `cw`
        constexpr wyne::variant<int, const char*> cw( cv );
        static_assert( 42 == wyne::get<int>( cw ), "" );
    }
}

TEST( Ctor_Copy, ValuelessByException ) {
    wyne::variant<int, move_thrower_t> v( 42 );
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    EXPECT_TRUE( v.valueless_by_exception() );
    wyne::variant<int, move_thrower_t> w( v );
    EXPECT_TRUE( w.valueless_by_exception() );
}

TEST( Ctor_Default, Variant ) {
    wyne::variant<int, std::string> v;
    EXPECT_EQ( 0, wyne::get<0>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int> cv{};
        static_assert( 0 == wyne::get<0>( cv ), "" );
    }
}

TEST( Ctor_Fwd, Direct ) {
    wyne::variant<int, std::string> v( 42 );
    EXPECT_EQ( 42, wyne::get<int>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( 42 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
    }
}

TEST( Ctor_Fwd, DirectConversion ) {
    wyne::variant<int, std::string> v( "42" );
    EXPECT_EQ( "42", wyne::get<std::string>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( 'A' );
        static_assert( 65 == wyne::get<int>( cv ), "" );
    }
}

TEST( Ctor_Fwd, CopyInitialization ) {
    wyne::variant<int, std::string> v = 42;
    EXPECT_EQ( 42, wyne::get<int>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv = 42;
        static_assert( 42 == wyne::get<int>( cv ), "" );
    }
}

TEST( Ctor_Fwd, CopyInitializationConversion ) {
    wyne::variant<int, std::string> v = "42";
    EXPECT_EQ( "42", wyne::get<std::string>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv = 'A';
        static_assert( 65 == wyne::get<int>( cv ), "" );
    }
}

TEST( Ctor_InPlace, IndexDirect ) {
    wyne::variant<int, std::string> v( wyne::in_place_index_t<0>{}, 42 );
    EXPECT_EQ( 42, wyne::get<0>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( wyne::in_place_index_t<0>{}, 42 );
        static_assert( 42 == wyne::get<0>( cv ), "" );
    }
}

TEST( Ctor_InPlace, IndexDirectDuplicate ) {
    wyne::variant<int, int> v( wyne::in_place_index_t<0>{}, 42 );
    EXPECT_EQ( 42, wyne::get<0>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, int> cv( wyne::in_place_index_t<0>{}, 42 );
        static_assert( 42 == wyne::get<0>( cv ), "" );
    }
}

TEST( Ctor_InPlace, IndexConversion ) {
    wyne::variant<int, std::string> v( wyne::in_place_index_t<1>{}, "42" );
    EXPECT_EQ( "42", wyne::get<1>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( wyne::in_place_index_t<0>{}, 1.1 );
        static_assert( 1 == wyne::get<0>( cv ), "" );
    }
}

TEST( Ctor_InPlace, IndexInitializerList ) {
    wyne::variant<int, std::string> v( wyne::in_place_index_t<1>{}, { '4', '2' } );
    EXPECT_EQ( "42", wyne::get<1>( v ) );
}

TEST( Ctor_InPlace, TypeDirect ) {
    wyne::variant<int, std::string> v( wyne::in_place_type_t<std::string>{}, "42" );
    EXPECT_EQ( "42", wyne::get<std::string>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( wyne::in_place_type_t<int>{}, 42 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
    }
}

TEST( Ctor_InPlace, TypeConversion ) {
    wyne::variant<int, std::string> v( wyne::in_place_type_t<int>{}, 42.5 );
    EXPECT_EQ( 42, wyne::get<int>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( wyne::in_place_type_t<int>{}, 42.5 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
    }
}

TEST( Ctor_InPlace, TypeInitializerList ) {
    wyne::variant<int, std::string> v( wyne::in_place_type_t<std::string>{}, { '4', '2' } );
    EXPECT_EQ( "42", wyne::get<std::string>( v ) );
}

TEST( Ctor_Move, Value ) {
    // `v`
    wyne::variant<int, std::string> v( "hello" );
    EXPECT_EQ( "hello", wyne::get<std::string>( v ) );
    // `w`
    wyne::variant<int, std::string> w( wyne::move( v ) );
    EXPECT_EQ( "hello", wyne::get<std::string>( w ) );

    /* constexpr */ {
        // `cv`
        constexpr wyne::variant<int, const char*> cv( 42 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
        // `cw`
        constexpr wyne::variant<int, const char*> cw( wyne::move( cv ) );
        static_assert( 42 == wyne::get<int>( cw ), "" );
    }
}

TEST( Ctor_Move, ValuelessByException ) {
    wyne::variant<int, move_thrower_t> v( 42 );
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    EXPECT_TRUE( v.valueless_by_exception() );
    wyne::variant<int, move_thrower_t> w( wyne::move( v ) );
    EXPECT_TRUE( w.valueless_by_exception() );
}

struct Obj {
    Obj( bool& dtor_called ) : dtor_called_( dtor_called ) {}
    ~Obj() { dtor_called_ = true; }
    bool& dtor_called_;
};  // Obj

TEST( Dtor, Value ) {
    bool dtor_called = false;
    // Construct/Destruct `Obj`.
    {
        wyne::variant<Obj> v( wyne::in_place_type_t<Obj>{}, dtor_called );
    }
    // Check that the destructor was called.
    EXPECT_TRUE( dtor_called );
}

TEST( Get, HoldsAlternative ) {
    wyne::variant<int, std::string> v( 42 );
    EXPECT_TRUE( wyne::holds_alternative<0>( v ) );
    EXPECT_FALSE( wyne::holds_alternative<1>( v ) );
    EXPECT_TRUE( wyne::holds_alternative<int>( v ) );
    EXPECT_FALSE( wyne::holds_alternative<std::string>( v ) );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( 42 );
        static_assert( wyne::holds_alternative<0>( cv ), "" );
        static_assert( !wyne::holds_alternative<1>( cv ), "" );
        static_assert( wyne::holds_alternative<int>( cv ), "" );
        static_assert( !wyne::holds_alternative<const char*>( cv ), "" );
    }
}

TEST( Get, MutVarMutType ) {
    wyne::variant<int> v( 42 );
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    // Check qualifier.
    EXPECT_EQ( LRef, get_qual( wyne::get<int>( v ) ) );
    EXPECT_EQ( RRef, get_qual( wyne::get<int>( wyne::move( v ) ) ) );
}

TEST( Get, MutVarConstType ) {
    wyne::variant<const int> v( 42 );
    EXPECT_EQ( 42, wyne::get<const int>( v ) );
    // Check qualifier.
    EXPECT_EQ( ConstLRef, get_qual( wyne::get<const int>( v ) ) );
    EXPECT_EQ( ConstRRef, get_qual( wyne::get<const int>( wyne::move( v ) ) ) );
}

TEST( Get, ConstVarMutType ) {
    const wyne::variant<int> v( 42 );
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    // Check qualifier.
    EXPECT_EQ( ConstLRef, get_qual( wyne::get<int>( v ) ) );
    EXPECT_EQ( ConstRRef, get_qual( wyne::get<int>( wyne::move( v ) ) ) );

    /* constexpr */ {
        constexpr wyne::variant<int> cv( 42 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
        // Check qualifier.
        static_assert( ConstLRef == get_qual( wyne::get<int>( cv ) ), "" );
        static_assert( ConstRRef == get_qual( wyne::get<int>( wyne::move( cv ) ) ), "" );
    }
}

TEST( Get, ConstVarConstType ) {
    const wyne::variant<const int> v( 42 );
    EXPECT_EQ( 42, wyne::get<const int>( v ) );
    // Check qualifier.
    EXPECT_EQ( ConstLRef, get_qual( wyne::get<const int>( v ) ) );
    EXPECT_EQ( ConstRRef, get_qual( wyne::get<const int>( wyne::move( v ) ) ) );

    /* constexpr */ {
        constexpr wyne::variant<const int> cv( 42 );
        static_assert( 42 == wyne::get<const int>( cv ), "" );
        // Check qualifier.
        static_assert( ConstLRef == get_qual( wyne::get<const int>( cv ) ), "" );
        static_assert( ConstRRef == get_qual( wyne::get<const int>( wyne::move( cv ) ) ), "" );
    }
}

TEST( Get, ValuelessByException ) {
    wyne::variant<int, move_thrower_t> v( 42 );
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    EXPECT_TRUE( v.valueless_by_exception() );
    EXPECT_THROW( wyne::get<int>( v ), wyne::bad_variant_access );
    EXPECT_THROW( wyne::get<move_thrower_t>( v ), wyne::bad_variant_access );
}

TEST( GetIf, MutVarMutType ) {
    wyne::variant<int> v( 42 );
    EXPECT_EQ( 42, *wyne::get_if<int>( &v ) );
    // Check qualifier.
    EXPECT_EQ( Ptr, get_qual( wyne::get_if<int>( &v ) ) );
}

TEST( GetIf, MutVarConstType ) {
    wyne::variant<const int> v( 42 );
    EXPECT_EQ( 42, *wyne::get_if<const int>( &v ) );
    // Check qualifier.
    EXPECT_EQ( ConstPtr, get_qual( wyne::get_if<const int>( &v ) ) );
}

TEST( GetIf, ConstVarMutType ) {
    const wyne::variant<int> v( 42 );
    EXPECT_EQ( 42, *wyne::get_if<int>( &v ) );
    // Check qualifier.
    EXPECT_EQ( ConstPtr, get_qual( wyne::get_if<int>( &v ) ) );

    /* constexpr */ {
        static constexpr wyne::variant<int> cv( 42 );
        static_assert( 42 == *wyne::get_if<int>( &cv ), "" );
        // Check qualifier.
        static_assert( ConstPtr == get_qual( wyne::get_if<int>( &cv ) ), "" );
    }
}

TEST( GetIf, ConstVarConstType ) {
    const wyne::variant<const int> v( 42 );
    EXPECT_EQ( 42, *wyne::get_if<const int>( &v ) );
    // Check qualifier.
    EXPECT_EQ( ConstPtr, get_qual( wyne::get_if<const int>( &v ) ) );

    /* constexpr */ {
        static constexpr wyne::variant<const int> cv( 42 );
        static_assert( 42 == *wyne::get_if<const int>( &cv ), "" );
        // Check qualifier.
        static_assert( ConstPtr == get_qual( wyne::get_if<const int>( &cv ) ), "" );
    }
}

TEST( GetIf, ValuelessByException ) {
    wyne::variant<int, move_thrower_t> v( 42 );
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    EXPECT_TRUE( v.valueless_by_exception() );
    EXPECT_EQ( nullptr, wyne::get_if<int>( &v ) );
    EXPECT_EQ( nullptr, wyne::get_if<move_thrower_t>( &v ) );
}

TEST( Rel, SameTypeSameValue ) {
    wyne::variant<int, std::string> v( 0 ), w( 0 );
    // `v` op `w`
    EXPECT_TRUE( v == w );
    EXPECT_FALSE( v != w );
    EXPECT_FALSE( v < w );
    EXPECT_FALSE( v > w );
    EXPECT_TRUE( v <= w );
    EXPECT_TRUE( v >= w );
    // `w` op `v`
    EXPECT_TRUE( w == v );
    EXPECT_FALSE( w != v );
    EXPECT_FALSE( w < v );
    EXPECT_FALSE( w > v );
    EXPECT_TRUE( w <= v );
    EXPECT_TRUE( w >= v );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( 0 ), cw( 0 );
        // `cv` op `cw`
        static_assert( cv == cw, "" );
        static_assert( !( cv != cw ), "" );
        static_assert( !( cv < cw ), "" );
        static_assert( !( cv > cw ), "" );
        static_assert( cv <= cw, "" );
        static_assert( cv >= cw, "" );
        // `cw` op `cv`
        static_assert( cw == cv, "" );
        static_assert( !( cw != cv ), "" );
        static_assert( !( cw < cv ), "" );
        static_assert( !( cw > cv ), "" );
        static_assert( cw <= cv, "" );
        static_assert( cw >= cv, "" );
    }
}

TEST( Rel, SameTypeDiffValue ) {
    wyne::variant<int, std::string> v( 0 ), w( 1 );
    // `v` op `w`
    EXPECT_FALSE( v == w );
    EXPECT_TRUE( v != w );
    EXPECT_TRUE( v < w );
    EXPECT_FALSE( v > w );
    EXPECT_TRUE( v <= w );
    EXPECT_FALSE( v >= w );
    // `w` op `v`
    EXPECT_FALSE( w == v );
    EXPECT_TRUE( w != v );
    EXPECT_FALSE( w < v );
    EXPECT_TRUE( w > v );
    EXPECT_FALSE( w <= v );
    EXPECT_TRUE( w >= v );

    /* constexpr */ {
        constexpr wyne::variant<int, const char*> cv( 0 ), cw( 1 );
        // `cv` op `cw`
        static_assert( !( cv == cw ), "" );
        static_assert( cv != cw, "" );
        static_assert( cv < cw, "" );
        static_assert( !( cv > cw ), "" );
        static_assert( cv <= cw, "" );
        static_assert( !( cv >= cw ), "" );
        // `cw` op `cv`
        static_assert( !( cw == cv ), "" );
        static_assert( cw != cv, "" );
        static_assert( !( cw < cv ), "" );
        static_assert( cw > cv, "" );
        static_assert( !( cw <= cv ), "" );
        static_assert( cw >= cv, "" );
    }
}

TEST( Rel, DiffTypeSameValue ) {
    wyne::variant<int, unsigned int> v( 0 ), w( 0u );
    // `v` op `w`
    EXPECT_FALSE( v == w );
    EXPECT_TRUE( v != w );
    EXPECT_TRUE( v < w );
    EXPECT_FALSE( v > w );
    EXPECT_TRUE( v <= w );
    EXPECT_FALSE( v >= w );
    // `w` op `v`
    EXPECT_FALSE( w == v );
    EXPECT_TRUE( w != v );
    EXPECT_FALSE( w < v );
    EXPECT_TRUE( w > v );
    EXPECT_FALSE( w <= v );
    EXPECT_TRUE( w >= v );

    /* constexpr */ {
        constexpr wyne::variant<int, unsigned int> cv( 0 ), cw( 0u );
        // `cv` op `cw`
        static_assert( !( cv == cw ), "" );
        static_assert( cv != cw, "" );
        static_assert( cv < cw, "" );
        static_assert( !( cv > cw ), "" );
        static_assert( cv <= cw, "" );
        static_assert( !( cv >= cw ), "" );
        // `cw` op `cv`
        static_assert( !( cw == cv ), "" );
        static_assert( cw != cv, "" );
        static_assert( !( cw < cv ), "" );
        static_assert( cw > cv, "" );
        static_assert( !( cw <= cv ), "" );
        static_assert( cw >= cv, "" );
    }
}

TEST( Rel, DiffTypeDiffValue ) {
    wyne::variant<int, unsigned int> v( 0 ), w( 1u );
    // `v` op `w`
    EXPECT_FALSE( v == w );
    EXPECT_TRUE( v != w );
    EXPECT_TRUE( v < w );
    EXPECT_FALSE( v > w );
    EXPECT_TRUE( v <= w );
    EXPECT_FALSE( v >= w );
    // `w` op `v`
    EXPECT_FALSE( w == v );
    EXPECT_TRUE( w != v );
    EXPECT_FALSE( w < v );
    EXPECT_TRUE( w > v );
    EXPECT_FALSE( w <= v );
    EXPECT_TRUE( w >= v );

    /* constexpr */ {
        constexpr wyne::variant<int, unsigned int> cv( 0 ), cw( 1u );
        // `cv` op `cw`
        static_assert( !( cv == cw ), "" );
        static_assert( cv != cw, "" );
        static_assert( cv < cw, "" );
        static_assert( !( cv > cw ), "" );
        static_assert( cv <= cw, "" );
        static_assert( !( cv >= cw ), "" );
        // `cw` op `cv`
        static_assert( !( cw == cv ), "" );
        static_assert( cw != cv, "" );
        static_assert( !( cw < cv ), "" );
        static_assert( cw > cv, "" );
        static_assert( !( cw <= cv ), "" );
        static_assert( cw >= cv, "" );
    }
}

// TEST( Rel, OneValuelessByException ) {
//     // `v` normal, `w` corrupted.
//     wyne::variant<int, move_thrower_t> v( 42 ), w( 42 );
//     EXPECT_THROW( w = move_thrower_t{}, MoveConstruction );
//     EXPECT_FALSE( v.valueless_by_exception() );
//     EXPECT_TRUE( w.valueless_by_exception() );
//     // `v` op `w`
//     EXPECT_FALSE( v == w );
//     EXPECT_TRUE( v != w );
//     EXPECT_FALSE( v < w );
//     EXPECT_TRUE( v > w );
//     EXPECT_FALSE( v <= w );
//     EXPECT_TRUE( v >= w );
// }

// TEST( Rel, BothValuelessByException ) {
//     // `v`, `w` both corrupted.
//     wyne::variant<int, move_thrower_t> v( 42 );
//     EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
//     wyne::variant<int, move_thrower_t> w( v );
//     EXPECT_TRUE( v.valueless_by_exception() );
//     EXPECT_TRUE( w.valueless_by_exception() );
//     // `v` op `w`
//     EXPECT_TRUE( v == w );
//     EXPECT_FALSE( v != w );
//     EXPECT_FALSE( v < w );
//     EXPECT_FALSE( v > w );
//     EXPECT_TRUE( v <= w );
//     EXPECT_TRUE( v >= w );
// }

TEST( Swap, Same ) {
    wyne::variant<int, std::string> v( "hello" );
    wyne::variant<int, std::string> w( "world" );
    // Check `v`.
    EXPECT_EQ( "hello", wyne::get<std::string>( v ) );
    // Check `w`.
    EXPECT_EQ( "world", wyne::get<std::string>( w ) );
    // Swap.
    using wyne::swap;
    swap( v, w );
    // Check `v`.
    EXPECT_EQ( "world", wyne::get<std::string>( v ) );
    // Check `w`.
    EXPECT_EQ( "hello", wyne::get<std::string>( w ) );
}

TEST( Swap, Different ) {
    wyne::variant<int, std::string> v( 42 );
    wyne::variant<int, std::string> w( "hello" );
    // Check `v`.
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    // Check `w`.
    EXPECT_EQ( "hello", wyne::get<std::string>( w ) );
    // Swap.
    using wyne::swap;
    swap( v, w );
    // Check `v`.
    EXPECT_EQ( "hello", wyne::get<std::string>( v ) );
    // Check `w`.
    EXPECT_EQ( 42, wyne::get<int>( w ) );
}

TEST( Swap, OneValuelessByException ) {
    // `v` normal, `w` corrupted.
    wyne::variant<int, move_thrower_t> v( 42 ), w( 42 );
    EXPECT_THROW( w = move_thrower_t{}, MoveConstruction );
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    EXPECT_TRUE( w.valueless_by_exception() );
    // Swap.
    using std::swap;
    swap( v, w );
    // Check `v`, `w`.
    EXPECT_TRUE( v.valueless_by_exception() );
    EXPECT_EQ( 42, wyne::get<int>( w ) );
}

TEST( Swap, BothValuelessByException ) {
    // `v`, `w` both corrupted.
    wyne::variant<int, move_thrower_t> v( 42 );
    EXPECT_THROW( v = move_thrower_t{}, MoveConstruction );
    wyne::variant<int, move_thrower_t> w( v );
    EXPECT_TRUE( v.valueless_by_exception() );
    EXPECT_TRUE( w.valueless_by_exception() );
    // Swap.
    using std::swap;
    swap( v, w );
    // Check `v`, `w`.
    EXPECT_TRUE( v.valueless_by_exception() );
    EXPECT_TRUE( w.valueless_by_exception() );
}

TEST( Swap, DtorsSame ) {
    struct Obj {
        Obj( size_t* dtor_count ) : dtor_count_( dtor_count ) {}
        Obj( const Obj& ) = default;
        Obj( Obj&& )      = default;
        ~Obj() { ++( *dtor_count_ ); }
        Obj&    operator=( const Obj& ) = default;
        Obj&    operator=( Obj&& )      = default;
        size_t* dtor_count_;
    };  // Obj
    size_t v_count = 0;
    size_t w_count = 0;
    {
        wyne::variant<Obj> v{ &v_count }, w{ &w_count };
        using std::swap;
        swap( v, w );
        // Calls `std::swap(Obj &lhs, Obj &rhs)`, with which we perform:
        // ```
        // {
        //   Obj temp(move(lhs));
        //   lhs = move(rhs);
        //   rhs = move(temp);
        // }  `++v_count` from `temp::~Obj()`.
        // ```
        EXPECT_EQ( 1u, v_count );
        EXPECT_EQ( 0u, w_count );
    }
    EXPECT_EQ( 2u, v_count );
    EXPECT_EQ( 1u, w_count );
}

namespace detail {

struct Obj {
    Obj( size_t* dtor_count ) : dtor_count_( dtor_count ) {}
    Obj( const Obj& ) = default;
    Obj( Obj&& )      = default;
    ~Obj() { ++( *dtor_count_ ); }
    Obj&    operator=( const Obj& ) = default;
    Obj&    operator=( Obj&& )      = default;
    size_t* dtor_count_;
};  // Obj

inline void swap( detail::Obj& lhs, detail::Obj& rhs ) {
    // std::cout << "lhs.dtor_count_ " << *lhs.dtor_count_ << std::endl;
    // std::cout << "rhs.dtor_count_ " << *rhs.dtor_count_ << std::endl;
    wyne::swap( lhs.dtor_count_, rhs.dtor_count_ );
}

// std::ostream& operator<<( std::ostream& os, const Obj& obj ) noexcept { return os << *( obj.dtor_count_ ); }

}  // namespace detail

namespace wyne {}  // namespace wyne

TEST( Swap, DtorsSameWithSwap ) {
    size_t v_count = 0;
    size_t w_count = 0;
    {
        wyne::variant<detail::Obj> v{ &v_count }, w{ &w_count };
        // std::cout << "testtttttttt" << std::endl;
        using wyne::swap;
        swap( v, w );
        // detail::Obj o1( &v_count ), o2( &w_count );
        // swap( o1, o2 );
        // std::cout << "testtttttttt" << std::endl;
        // Calls `detail::swap(Obj &lhs, Obj &rhs)`, with which doesn't call any destructors.
        EXPECT_EQ( 0u, v_count );
        EXPECT_EQ( 0u, w_count );
    }
    EXPECT_EQ( 1u, v_count );
    EXPECT_EQ( 1u, w_count );
}

TEST( Swap, DtorsDifferent ) {
    struct V {
        V( size_t* dtor_count ) : dtor_count_( dtor_count ) {}
        V( const V& ) = default;
        V( V&& )      = default;
        ~V() { ++( *dtor_count_ ); }
        V&      operator=( const V& ) = default;
        V&      operator=( V&& )      = default;
        size_t* dtor_count_;
    };  // V
    struct W {
        W( size_t* dtor_count ) : dtor_count_( dtor_count ) {}
        W( const W& ) = default;
        W( W&& )      = default;
        ~W() { ++( *dtor_count_ ); }
        W&      operator=( const W& ) = default;
        W&      operator=( W&& )      = default;
        size_t* dtor_count_;
    };  // W
    size_t v_count = 0;
    size_t w_count = 0;
    {
        wyne::variant<V, W> v{ wyne::in_place_type_t<V>{}, &v_count };
        wyne::variant<V, W> w{ wyne::in_place_type_t<W>{}, &w_count };
        using std::swap;
        swap( v, w );
        EXPECT_EQ( 1u, v_count );
        EXPECT_EQ( 2u, w_count );
    }
    EXPECT_EQ( 2u, v_count );
    EXPECT_EQ( 3u, w_count );
}

TEST( Visit, MutVarMutType ) {
    wyne::variant<int> v( 42 );
    // Check `v`.
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    // Check qualifier.
    EXPECT_EQ( LRef, wyne::visit( get_qual, v ) );
    EXPECT_EQ( RRef, wyne::visit( get_qual, wyne::move( v ) ) );
}

TEST( Visit, MutVarConstType ) {
    wyne::variant<const int> v( 42 );
    EXPECT_EQ( 42, wyne::get<const int>( v ) );
    // Check qualifier.
    EXPECT_EQ( ConstLRef, wyne::visit( get_qual, v ) );
    EXPECT_EQ( ConstRRef, wyne::visit( get_qual, wyne::move( v ) ) );
}

TEST( Visit, ConstVarMutType ) {
    const wyne::variant<int> v( 42 );
    EXPECT_EQ( 42, wyne::get<int>( v ) );
    // Check qualifier.
    EXPECT_EQ( ConstLRef, wyne::visit( get_qual, v ) );
    EXPECT_EQ( ConstRRef, wyne::visit( get_qual, wyne::move( v ) ) );

    /* constexpr */ {
        constexpr wyne::variant<int> cv( 42 );
        static_assert( 42 == wyne::get<int>( cv ), "" );
        // Check qualifier.
        static_assert( ConstLRef == wyne::visit( get_qual, cv ), "" );
        static_assert( ConstRRef == wyne::visit( get_qual, wyne::move( cv ) ), "" );
    }
}

TEST( Visit, ConstVarConstType ) {
    const wyne::variant<const int> v( 42 );
    EXPECT_EQ( 42, wyne::get<const int>( v ) );
    // Check qualifier.
    EXPECT_EQ( ConstLRef, wyne::visit( get_qual, v ) );
    EXPECT_EQ( ConstRRef, wyne::visit( get_qual, wyne::move( v ) ) );

    /* constexpr */ {
        constexpr wyne::variant<const int> cv( 42 );
        static_assert( 42 == wyne::get<const int>( cv ), "" );
        // Check qualifier.
        static_assert( ConstLRef == wyne::visit( get_qual, cv ), "" );
        static_assert( ConstRRef == wyne::visit( get_qual, wyne::move( cv ) ), "" );
    }
}

struct concat {
    template <typename... Args>
    std::string operator()( const Args&... args ) const {
        std::ostringstream strm;
        std::initializer_list<int>( { ( strm << args, 0 )... } );
        return wyne::move( strm ).str();
    }
};

TEST( Visit, Zero ) { EXPECT_EQ( "", wyne::visit( concat{} ) ); }

TEST( Visit_Homogeneous, Double ) {
    wyne::variant<int, std::string> v( "hello" ), w( "world!" );
    EXPECT_EQ( "helloworld!", wyne::visit( concat{}, v, w ) );

    /* constexpr */ {
        constexpr wyne::variant<int, double> cv( 101 ), cw( 202 ), cx( 3.3 );
        struct add_ints {
            constexpr int operator()( int lhs, int rhs ) const { return lhs + rhs; }
            constexpr int operator()( int lhs, double ) const { return lhs; }
            constexpr int operator()( double, int rhs ) const { return rhs; }
            constexpr int operator()( double, double ) const { return 0; }
        };  // add
        static_assert( 303 == wyne::visit( add_ints{}, cv, cw ), "" );
        static_assert( 202 == wyne::visit( add_ints{}, cw, cx ), "" );
        static_assert( 101 == wyne::visit( add_ints{}, cx, cv ), "" );
        static_assert( 0 == wyne::visit( add_ints{}, cx, cx ), "" );
    }
}

TEST( Visit_Homogeneous, Quintuple ) {
    wyne::variant<int, std::string> v( 101 ), w( "+" ), x( 202 ), y( "=" ), z( 303 );
    EXPECT_EQ( "101+202=303", wyne::visit( concat{}, v, w, x, y, z ) );
}

TEST( Visit_Heterogeneous, Double ) {
    wyne::variant<int, std::string>    v( "hello" );
    wyne::variant<double, const char*> w( "world!" );
    EXPECT_EQ( "helloworld!", wyne::visit( concat{}, v, w ) );
}

TEST( Visit_Heterogenous, Quintuple ) {
    wyne::variant<int, double>                    v( 101 );
    wyne::variant<const char*>                    w( "+" );
    wyne::variant<bool, std::string, int>         x( 202 );
    wyne::variant<char, std::string, const char*> y( '=' );
    wyne::variant<long, short>                    z( 303L );
    EXPECT_EQ( "101+202=303", wyne::visit( concat{}, v, w, x, y, z ) );
}
