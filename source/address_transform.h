#ifndef DSR_ADDRESS_TRANSFORM_H
#define DSR_ADDRESS_TRANSFORM_H

namespace roads
{
    namespace detail
    {
        // Base class template that provides typedefs and
        // a static function needed in register access
        template <typename ResultType> struct apply_op
        {
            typedef ResultType* addr_type;
            typedef ResultType result_type;
            static result_type apply(addr_type addr) { return *addr; }
        };

        // Returns a reference to a register
        template <typename ResultType> struct apply_op<ResultType&>
        {
            typedef ResultType* addr_type;
            typedef ResultType& result_type;
            static result_type apply(addr_type addr) { return *addr; }
        };

        // Returns a pointer to a register (not used)
        template <typename ResultType> struct apply_op<ResultType*>
        {
            typedef ResultType* addr_type;
            typedef ResultType* result_type;
            static result_type apply(addr_type addr) { return addr; }
        };

        // Returns a reference-to-array to a set of contiguous registers
        template <typename ResultType, std::size_t N> struct apply_op<ResultType[N]>
        {
            typedef ResultType ArrayType[N];

            typedef ResultType* addr_type;
            typedef ArrayType&  result_type;
            static result_type apply(addr_type addr) { return *reinterpret_cast<ArrayType*>(addr); }
        };
    }
}

#endif // DSR_ADDRESS_TRANSFORM_H
