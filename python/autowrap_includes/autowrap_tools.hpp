
inline char * _cast_const_away(const char *p)
{
    return const_cast<char *>(p);
}

template<class A> void _iadd(A * a1, const A * a2)
{
    (*a1) += (*a2);
}

namespace autowrap {

    template <class X>
    class AutowrapRefHolder {

        private:

            X& _ref;

        public:

            AutowrapRefHolder(X &ref): _ref(ref) 
            {
            }

            X& get()
            {
                return _ref;
            }

            void assign(const X & refneu)
            {
                _ref = refneu;
            }

    };

};
