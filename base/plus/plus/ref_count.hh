/*
	ref_count.hh
	------------
*/

#ifndef PLUS_REFCOUNT_HH
#define PLUS_REFCOUNT_HH


namespace plus
{
	
	template < class T > struct destroyer
	{
		static void apply( T* x )
		{
			delete x;
		}
	};
	
	template < class T >
	inline void destroy( T* x )
	{
		destroyer< T >::apply( x );
	}
	
	class ref_count_base
	{
		private:
			mutable unsigned long its_n;
			
			// Non-copyable
			ref_count_base           ( const ref_count_base& );
			ref_count_base& operator=( const ref_count_base& );
			
		protected:
			// Protected constructor to prevent slicing
			ref_count_base() : its_n( 0 )
			{
			}
			
			unsigned long release() const
			{
				return --its_n;
			}
		
		public:
			friend unsigned long intrusive_ptr_ref_count( const ref_count_base* count )
			{
				return count->its_n;
			}
		
		private:
			friend void intrusive_ptr_add_ref( const ref_count_base* count )
			{
				++count->its_n;
			}
	};
	
	template < class Derived >
	class ref_count : public ref_count_base
	{
		private:
			// Hide the protected release() from view
			unsigned long release() const
			{
				return ref_count_base::release();
			}
			
			friend void intrusive_ptr_release( const Derived* derived )
			{
				if ( derived->release() == 0 )
				{
					destroy( derived );
				}
			}
		
		protected:
			// Protected destructor to prevent slicing
			~ref_count()
			{
			}
	};
	
}

#endif

