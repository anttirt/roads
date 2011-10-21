#ifndef DSR_CONTAINER_WRAPPER_H
#define DSR_CONTAINER_WRAPPER_H

#define DSR_WRAP_GETCONT GetFn()(static_cast<Wrapper&>(*this))
#define DSR_CWRAP_GETCONT ConstGetFn()(static_cast<Wrapper const&>(*this))

template <
    typename Wrapper,
    typename Container,
    typename GetFn,
    typename ConstGetFn
>
struct wrapper_base
{
    typedef typename Container::value_type value_type;
    typedef typename Container::reference reference;
    typedef typename Container::difference_type difference_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;
    
    iterator begin() { return DSR_WRAP_GETCONT.begin(); }
    iterator end() { return DSR_WRAP_GETCONT.end(); }
    const_iterator begin() const { return DSR_CWRAP_GETCONT.begin(); }
    const_iterator end() const { return DSR_CWRAP_GETCONT.end(); }

    bool empty() const { return DSR_CWRAP_GETCONT.empty(); }

    size_type size() const { return DSR_CWRAP_GETCONT.size(); }
    size_type max_size() const { return DSR_CWRAP_GETCONT.max_size(); }
};

template <
    typename Wrapper,
    typename Container,
    typename GetFn,
    typename ConstGetFn
>
struct reversible_wrapper_base
    : wrapper_base<Wrapper, Container, GetFn, ConstGetFn>
{
    typedef wrapper_base<Wrapper, Container, GetFn, ConstGetFn> base;
    typedef typename base::value_type value_type;
    typedef typename base::reference reference;
    typedef typename base::difference_type difference_type;
    typedef typename base::size_type size_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename Container::reverse_iterator reverse_iterator;
    typedef typename Container::const_reverse_iterator const_reverse_iterator;

    reverse_iterator rbegin() { return DSR_WRAP_GETCONT.rbegin(); }
    reverse_iterator rend() { return DSR_WRAP_GETCONT.rend(); }
    const_reverse_iterator rbegin() const { return DSR_CWRAP_GETCONT.rbegin(); }
    const_reverse_iterator rend() const { return DSR_CWRAP_GETCONT.rend(); }
};

template <
    typename Wrapper,
    typename Container,
    typename GetFn,
    typename ConstGetFn
>
struct sequence_wrapper_base
    : reversible_wrapper_base<Wrapper, Container, GetFn, ConstGetFn>
{
    typedef reversible_wrapper_base<Wrapper, Container, GetFn, ConstGetFn> base;
    typedef typename base::value_type value_type;
    typedef typename base::reference reference;
    typedef typename base::difference_type difference_type;
    typedef typename base::size_type size_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename base::reverse_iterator reverse_iterator;
    typedef typename base::const_reverse_iterator const_reverse_iterator;

    void insert(iterator position, value_type value) { DSR_WRAP_GETCONT.insert(position, value); }
    void insert(iterator position, size_type count, value_type value) { DSR_WRAP_GETCONT.insert(position, count, value); }
    template <typename FwdIter>
    void insert(iterator position, FwdIter beg_, FwdIter end_) { DSR_WRAP_GETCONT.insert(position, beg_, end_); }

    iterator erase(iterator position) { return DSR_WRAP_GETCONT.erase(position); }
    iterator erase(iterator beg_, iterator end_) { return DSR_WRAP_GETCONT.erase(beg_, end_); }

    void clear() { DSR_WRAP_GETCONT.clear(); }
};

#undef DSR_WRAP_GETCONT
#undef DSR_CWRAP_GETCONT

#endif // DSR_CONTAINER_WRAPPER_H
