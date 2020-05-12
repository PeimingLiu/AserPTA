//
// Created by peiming on 11/8/19.
//
#ifndef ASER_PTA_ITERATORS_H
#define ASER_PTA_ITERATORS_H

#include <llvm/ADT/iterator.h>

namespace aser {

template <typename _Tp>
struct const_pointer {
    typedef const typename std::remove_pointer<_Tp>::type *type;
};

template <typename WrapperIteratorT,
          typename ValueT = typename std::conditional<
              std::is_const<typename std::remove_pointer<typename WrapperIteratorT::iterator_type>::type>::value,
              typename const_pointer<typename std::iterator_traits<WrapperIteratorT>::value_type::pointer>::type,
              typename std::iterator_traits<WrapperIteratorT>::value_type::pointer>::type>
class UniquePtrIterator
    : public llvm::iterator_adaptor_base<UniquePtrIterator<WrapperIteratorT>, WrapperIteratorT,
                                         typename std::iterator_traits<WrapperIteratorT>::iterator_category, ValueT> {
private:
    using BaseT =
        llvm::iterator_adaptor_base<UniquePtrIterator<WrapperIteratorT>, WrapperIteratorT,
                                    typename std::iterator_traits<WrapperIteratorT>::iterator_category, ValueT>;

public:
    UniquePtrIterator() = default;

    explicit UniquePtrIterator(const WrapperIteratorT &i) : BaseT(i) {}
    explicit UniquePtrIterator(WrapperIteratorT &&i) : BaseT(std::move(i)) {}

    ValueT operator*() const { return this->I->get(); }
};

template <typename WrapperIteratorT,
          typename ValueT = typename std::iterator_traits<WrapperIteratorT>::value_type::second_type>
class PairSecondIterator
    : public llvm::iterator_adaptor_base<PairSecondIterator<WrapperIteratorT>, WrapperIteratorT,
                                         typename std::iterator_traits<WrapperIteratorT>::iterator_category, ValueT> {
private:
    using BaseT =
        llvm::iterator_adaptor_base<PairSecondIterator<WrapperIteratorT>, WrapperIteratorT,
                                    typename std::iterator_traits<WrapperIteratorT>::iterator_category, ValueT>;

public:
    PairSecondIterator() = default;

    explicit PairSecondIterator(const WrapperIteratorT &i) : BaseT(i) {}

    explicit PairSecondIterator(WrapperIteratorT &&i) : BaseT(std::move(i)) {}

    ValueT operator*() const { return this->I->second; }
};

// TODO: this is a bad implementation, just use llvm::concat_iterator
// iterator that concat N iterator together
template <typename Wrapped, int N>
struct ConcatIterator : public ConcatIterator<Wrapped, N - 1> {
    using super = ConcatIterator<Wrapped, N - 1>;
    using self = ConcatIterator<Wrapped, N>;

    using ValueT = typename std::iterator_traits<Wrapped>::value_type;
    using ReferenceT = typename std::iterator_traits<Wrapped>::reference;

    Wrapped cur;
    Wrapped end;

    template <typename ... Args>
    ConcatIterator(Wrapped cur, Wrapped end, Args && ...args) :
        super(std::forward<Args>(args)...),
        cur(cur), end(end) {}

    inline self &operator++() {
        if (cur == end) {
            static_cast<super *>(this)->operator++();
        } else {
            cur++;
        }
        return *this;
    };

    self operator++(int) {
        self tmp = *static_cast<self *>(this);
        if (cur == end) {
            ++*static_cast<super *>(this);
        } else {
            ++cur;
        }
        return tmp;
    }

    auto operator*() -> decltype(*cur) {
        if (cur == end) {
            return static_cast<super *>(this)->operator*();
        } else {
            return *cur;
        }
    }

    inline bool operator!=(const self &rhs) { return !this->operator==(rhs); }

    inline bool operator==(const self &rhs) { return cur == rhs.cur && ((super *)this)->operator==((super &)rhs); }

    auto operator-> () -> decltype(cur.operator->()) {
        if (cur == end) {
            return static_cast<super *>(this)->operator->();
        } else {
            return cur.operator->();
        }
    }
};

template <typename Wrapped>
struct ConcatIterator<Wrapped, 1>
    : public std::iterator<std::forward_iterator_tag, typename std::iterator_traits<Wrapped>::value_type> {
    using self = ConcatIterator<Wrapped, 1>;
    using ValueT = typename std::iterator_traits<Wrapped>::value_type;
    using ReferenceT = typename std::iterator_traits<Wrapped>::reference;

    Wrapped cur;
    Wrapped end;

    ConcatIterator(Wrapped cur, Wrapped end)
        : cur(cur), end(end) {}

    inline self &operator++() {
        cur++;
        return *this;
    };

    self operator++(int) {
        self tmp = *static_cast<self *>(this);
        ++cur;
        return tmp;
    }

    inline ReferenceT operator*() { return *cur; }

    inline bool operator!=(const ConcatIterator<Wrapped, 1> &rhs) { return !this->operator==(rhs); }

    inline bool operator==(const ConcatIterator<Wrapped, 1> &rhs) { return cur == rhs.cur; }

    auto operator-> () -> decltype(cur.operator->()) { return cur.operator->(); }
};

// ASSUMPTION: E (a enum) and N are convertible
template <typename Wrapped, int N, typename E>
struct ConcatIteratorWithTag : public ConcatIteratorWithTag<Wrapped, N - 1, E> {
    using super = ConcatIteratorWithTag<Wrapped, N - 1, E>;
    using self = ConcatIteratorWithTag<Wrapped, N, E>;

    using ValueT = const std::pair<E, typename std::iterator_traits<Wrapped>::value_type>;
    using ReferenceT = ValueT;
    using PointerT = ValueT;

    Wrapped cur;
    Wrapped end;

    template <typename ... Args>
    ConcatIteratorWithTag(Wrapped cur, Wrapped end, Args && ...args) :
        super(std::forward<Args>(args)...),
        cur(cur), end(end) {}

    inline self &operator++() {
        if (cur == end) {
            static_cast<super *>(this)->operator++();
        } else {
            cur++;
        }
        return *this;
    };

    self operator++(int) {
        self tmp = *static_cast<self *>(this);
        if (cur == end) {
            ++*static_cast<super *>(this);
        } else {
            ++cur;
        }
        return tmp;
    }

    ReferenceT operator*() {
        if (cur == end) {
            return static_cast<super *>(this)->operator*();
        } else {
            return std::make_pair(static_cast<E>(N - 1), *cur);
        }
    }

    inline bool operator!=(const self &rhs) { return !this->operator==(rhs); }

    inline bool operator==(const self &rhs) {
        return cur == rhs.cur && static_cast<super *>(this)->operator==((super &)rhs);
    }
};

template <typename Wrapped, typename E>
struct ConcatIteratorWithTag<Wrapped, 1, E>
    : public std::iterator<std::forward_iterator_tag,
                           const std::pair<E, typename std::iterator_traits<Wrapped>::value_type>,
                           typename std::iterator_traits<Wrapped>::difference_type,
                           const std::pair<E, typename std::iterator_traits<Wrapped>::value_type>,
                           const std::pair<E, typename std::iterator_traits<Wrapped>::value_type>> {
    using self = ConcatIteratorWithTag<Wrapped, 1, E>;
    using ValueT = const std::pair<E, typename std::iterator_traits<Wrapped>::value_type>;
    using ReferenceT = ValueT;
    using PointerT = ValueT;

    Wrapped cur;
    Wrapped end;

    ConcatIteratorWithTag(Wrapped cur, Wrapped end)
        : cur(cur), end(end) {}

    inline self &operator++() {
        cur++;
        return *this;
    };

    self operator++(int) {
        self tmp = *static_cast<self *>(this);
        ++cur;
        return tmp;
    }

    inline ValueT operator*() { return std::make_pair(static_cast<E>(0), *cur); }

    inline bool operator!=(const self &rhs) { return !this->operator==(rhs); }

    inline bool operator==(const self &rhs) { return cur == rhs.cur; }
};

}  // namespace aser

#endif