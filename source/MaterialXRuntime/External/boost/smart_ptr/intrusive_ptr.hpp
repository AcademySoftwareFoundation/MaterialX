#ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
#define BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED

//
//  intrusive_ptr.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/ for documentation.
//

#define BOOST_SP_NOEXCEPT noexcept
#define BOOST_CONSTEXPR constexpr

namespace boost
{
    namespace detail
    {
        typedef decltype(nullptr) sp_nullptr_t;
    }
}

namespace boost
{

//
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//  
//      void intrusive_ptr_add_ref(T * p);
//      void intrusive_ptr_release(T * p);
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template<class T> class intrusive_ptr
{
private:

    typedef intrusive_ptr this_type;

public:

    typedef T element_type;

    BOOST_CONSTEXPR intrusive_ptr() BOOST_SP_NOEXCEPT : px( 0 )
    {
    }

    explicit intrusive_ptr( T * p, bool add_ref = true ): px( p )
    {
        if( px != 0 && add_ref ) intrusive_ptr_add_ref( px );
    }

    template<class U>
    intrusive_ptr( intrusive_ptr<U> const & rhs )
    : px( rhs.get() )
    {
        if( px != 0 ) intrusive_ptr_add_ref( px );
    }

    intrusive_ptr(intrusive_ptr const & rhs): px( rhs.px )
    {
        if( px != 0 ) intrusive_ptr_add_ref( px );
    }

    ~intrusive_ptr()
    {
        if( px != 0 ) intrusive_ptr_release( px );
    }

    template<class U> intrusive_ptr & operator=(intrusive_ptr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

// Move support

    intrusive_ptr(intrusive_ptr && rhs) BOOST_SP_NOEXCEPT : px( rhs.px )
    {
        rhs.px = 0;
    }

    intrusive_ptr & operator=(intrusive_ptr && rhs) BOOST_SP_NOEXCEPT
    {
        this_type( static_cast< intrusive_ptr && >( rhs ) ).swap(*this);
        return *this;
    }

    template<class U> friend class intrusive_ptr;

    template<class U>
    intrusive_ptr(intrusive_ptr<U> && rhs)
    : px( rhs.px )
    {
        rhs.px = 0;
    }

    template<class U>
    intrusive_ptr & operator=(intrusive_ptr<U> && rhs) BOOST_SP_NOEXCEPT
    {
        this_type( static_cast< intrusive_ptr<U> && >( rhs ) ).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(intrusive_ptr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset()
    {
        this_type().swap( *this );
    }

    void reset( T * rhs )
    {
        this_type( rhs ).swap( *this );
    }

    void reset( T * rhs, bool add_ref )
    {
        this_type( rhs, add_ref ).swap( *this );
    }

    T * get() const BOOST_SP_NOEXCEPT
    {
        return px;
    }

    T * detach() BOOST_SP_NOEXCEPT
    {
        T * ret = px;
        px = 0;
        return ret;
    }

    T & operator*() const BOOST_SP_NOEXCEPT
    {
        return *px;
    }

    T * operator->() const BOOST_SP_NOEXCEPT
    {
        return px;
    }

    explicit operator bool() const BOOST_SP_NOEXCEPT
    {
        return px != 0;
    }

    void swap(intrusive_ptr & rhs) BOOST_SP_NOEXCEPT
    {
        T * tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

private:

    T * px;
};

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b) BOOST_SP_NOEXCEPT
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b) BOOST_SP_NOEXCEPT
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, U * b) BOOST_SP_NOEXCEPT
{
    return a.get() == b;
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, U * b) BOOST_SP_NOEXCEPT
{
    return a.get() != b;
}

template<class T, class U> inline bool operator==(T * a, intrusive_ptr<U> const & b) BOOST_SP_NOEXCEPT
{
    return a == b.get();
}

template<class T, class U> inline bool operator!=(T * a, intrusive_ptr<U> const & b) BOOST_SP_NOEXCEPT
{
    return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b) BOOST_SP_NOEXCEPT
{
    return a.get() != b.get();
}

#endif

#if !defined( BOOST_NO_CXX11_NULLPTR )

template<class T> inline bool operator==( intrusive_ptr<T> const & p, boost::detail::sp_nullptr_t ) BOOST_SP_NOEXCEPT
{
    return p.get() == 0;
}

template<class T> inline bool operator==( boost::detail::sp_nullptr_t, intrusive_ptr<T> const & p ) BOOST_SP_NOEXCEPT
{
    return p.get() == 0;
}

template<class T> inline bool operator!=( intrusive_ptr<T> const & p, boost::detail::sp_nullptr_t ) BOOST_SP_NOEXCEPT
{
    return p.get() != 0;
}

template<class T> inline bool operator!=( boost::detail::sp_nullptr_t, intrusive_ptr<T> const & p ) BOOST_SP_NOEXCEPT
{
    return p.get() != 0;
}

#endif

template<class T> inline bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b) BOOST_SP_NOEXCEPT
{
    return std::less<T *>()(a.get(), b.get());
}

template<class T> void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs) BOOST_SP_NOEXCEPT
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(intrusive_ptr<T> const & p) BOOST_SP_NOEXCEPT
{
    return p.get();
}

// pointer casts

template<class T, class U> intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

template<class T, class U> intrusive_ptr<T> static_pointer_cast( intrusive_ptr<U> && p ) BOOST_SP_NOEXCEPT
{
    return intrusive_ptr<T>( static_cast<T*>( p.detach() ), false );
}

template<class T, class U> intrusive_ptr<T> const_pointer_cast( intrusive_ptr<U> && p ) BOOST_SP_NOEXCEPT
{
    return intrusive_ptr<T>( const_cast<T*>( p.detach() ), false );
}

template<class T, class U> intrusive_ptr<T> dynamic_pointer_cast( intrusive_ptr<U> && p ) BOOST_SP_NOEXCEPT
{
    T * p2 = dynamic_cast<T*>( p.get() );

    intrusive_ptr<T> r( p2, false );

    if( p2 ) p.detach();

    return r;
}

#endif // defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

// hash_value

template< class T > struct hash;

template< class T > size_t hash_value( boost::intrusive_ptr<T> const & p ) BOOST_SP_NOEXCEPT
{
    return std::hash< T* >()(p.get());
//    return boost::hash< T* >()(p.get());
}

} // namespace boost

#endif  // #ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
