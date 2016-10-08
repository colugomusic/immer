
#pragma once

#include <immer/detail/rbbits.hpp>

#include <utility>
#include <cassert>

namespace immer {
namespace detail {

template <typename NodeT>
struct empty_regular_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    node_t* node_;

    auto count() const { return 0; }
    auto node()  const { return node_; }
    auto shift() const { return 0; }
    auto size()  const { return 0; }

    template <typename Visitor>
    void each(Visitor&&) {}

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vi(*this, v);
        });
    }
};

template <typename NodeT>
empty_regular_rbpos<NodeT> make_empty_regular_rbpos(NodeT* node)
{
    assert(node);
    return {node};
}

template <typename NodeT>
struct empty_leaf_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    node_t* node_;

    auto count() const { return 0; }
    auto node()  const { return node_; }
    auto shift() const { return 0; }
    auto size()  const { return 0; }

    template <typename ...Visitor>
    void each(Visitor&&...) {}

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vl(*this, v);
        });
    }
};

template <typename NodeT>
empty_leaf_rbpos<NodeT> make_empty_leaf_rbpos(NodeT* node)
{
    assert(node);
    return {node};
}

template <typename NodeT>
struct leaf_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    node_t* node_;
    unsigned count_;

    auto count() const { return count_; }
    auto node()  const { return node_; }
    auto size()  const { return count_; }
    auto shift() const { return 0; }

    template <typename ...Visitor>
    void each(Visitor&&...) {}

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vl(*this, v);
        });
    }
};

template <typename NodeT>
leaf_rbpos<NodeT> make_leaf_rbpos(NodeT* node, unsigned count)
{
    assert(node);
    assert(count > 0);
    return {node, count};
}

template <typename NodeT>
struct full_leaf_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    node_t* node_;

    auto count() const { return branches<bits>; }
    auto node()  const { return node_; }
    auto size()  const { return branches<bits>; }
    auto shift() const { return 0; }

    template <typename ...Visitor>
    void each(Visitor&&...) {}

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vl(*this, v);
        });
    }
};

template <typename NodeT>
full_leaf_rbpos<NodeT> make_full_leaf_rbpos(NodeT* node)
{
    assert(node);
    return {node};
}

template <typename NodeT>
struct regular_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    node_t* node_;
    unsigned shift_;
    std::size_t size_;

    auto count() const { return (((size_ - 1) >> shift_) & mask<bits>) + 1; }
    auto node()  const { return node_; }
    auto size()  const { return size_; }
    auto shift() const { return shift_; }

    template <typename Visitor>
    void each(Visitor&& v)
    {
        if (shift_ == bits) {
            auto p = node_->inner();
            auto e = p + (((size_ - 1) >> bits) & mask<bits>);
            for (; p != e; ++p)
                make_full_leaf_rbpos(*p).visit(v);
            make_leaf_rbpos(*p, ((size_ - 1) & mask<bits>) + 1).visit(v);
        } else {
            auto p = node_->inner();
            auto e = p + (((size_ - 1) >> shift_) & mask<bits>);
            auto ss = shift_ - bits;
            for (; p != e; ++p)
                make_full_rbpos(*p, ss).visit(v);
            make_regular_rbpos(*p, ss, size_).visit(v);
        }
    }

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vi(*this, v);
        });
    }
};

template <typename NodeT>
regular_rbpos<NodeT> make_regular_rbpos(NodeT* node,
                                        unsigned shift,
                                        std::size_t size)
{
    assert(node);
    assert(shift >= NodeT::bits);
    assert(size > 0);
    return {node, shift, size};
}

template <typename NodeT>
struct full_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    node_t* node_;
    unsigned shift_;

    auto count() const { return branches<bits>; }
    auto node()  const { return node_; }
    auto size()  const { return 1 << shift_; }
    auto shift() const { return shift_; }

    template <typename Visitor>
    void each(Visitor&& v)
    {
        if (shift_ == bits) {
            auto p = node_->inner();
            auto e = p + branches<bits>;
            for (; p != e; ++p)
                make_full_leaf_rbpos(*p).visit(v);
        } else {
            auto p = node_->inner();
            auto e = p + branches<bits>;
            auto ss = shift_ - bits;
            for (; p != e; ++p)
                make_full_rbpos(*p, ss).visit(v);
        }
    }

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vi(*this, v);
        });
    }
};

template <typename NodeT>
full_rbpos<NodeT> make_full_rbpos(NodeT* node,
                                  unsigned shift)
{
    assert(node);
    assert(shift >= NodeT::bits);
    return {node, shift};
}

template <typename NodeT>
struct relaxed_rbpos
{
    static constexpr auto bits = NodeT::bits;
    using node_t = NodeT;
    using relaxed_t = typename NodeT::relaxed_t;
    node_t* node_;
    unsigned shift_;
    relaxed_t* relaxed_;

    auto count() const { return relaxed_->count; }
    auto node()  const { return node_; }
    auto size()  const { return relaxed_->sizes[relaxed_->count - 1]; }
    auto shift() const { return shift_; }

    template <typename Visitor>
    void each(Visitor&& v)
    {
        if (shift_ == bits) {
            auto p = node_->inner();
            auto s = std::size_t{};
            for (auto i = 0u; i < relaxed_->count; ++i) {
                make_leaf_rbpos(p[i], relaxed_->sizes[i] - s).visit(v);
                s = relaxed_->sizes[i];
            }
        } else {
            auto p = node_->inner();
            auto s = std::size_t{};
            auto ss = shift_ - bits;
            for (auto i = 0u; i < relaxed_->count; ++i) {
                visit_maybe_relaxed(p[i], ss, relaxed_->sizes[i] - s, v);
                s = relaxed_->sizes[i];
            }
        }
    }

    template <typename Visitor>
    auto visit(Visitor&& v)
    {
        return v([&] (auto&& vr, auto&& vi, auto&& vl) {
            return vr(*this, v);
        });
    }
};

template <typename NodeT>
relaxed_rbpos<NodeT> make_relaxed_rbpos(NodeT* node,
                                        unsigned shift,
                                        typename NodeT::relaxed_t* relaxed)
{
    assert(node);
    assert(relaxed);
    assert(shift >= NodeT::bits);
    return {node, shift, relaxed};
}

template <typename NodeT, typename Visitor>
auto visit_maybe_relaxed(NodeT* node, unsigned shift, std::size_t size,
                         Visitor&& v)
{
    assert(node);
    auto relaxed = node->relaxed();
    if (relaxed) {
        assert(size == relaxed->sizes[relaxed->count - 1]);
        return make_relaxed_rbpos(node, shift, relaxed).visit(v);
    } else {
        return make_regular_rbpos(node, shift, size).visit(v);
    }
}

template <typename GenericVisitor>
auto make_visitor(GenericVisitor v)
{
    return
        [=] (auto&& op) {
            return std::forward<decltype(op)>(op) (
                v, v, v);
        };
}

template <typename InnerVisitor, typename LeafVisitor>
auto make_visitor(InnerVisitor vi, LeafVisitor vl)
{
    return
        [=] (auto&& op) {
            return std::forward<decltype(op)>(op) (
                vi, vi, vl);
        };
}

template <typename RelaxedVisitor, typename InnerVisitor, typename LeafVisitor>
auto make_visitor(RelaxedVisitor vr, InnerVisitor vi, LeafVisitor vl)
{
    return
        [=] (auto&& op) {
            return std::forward<decltype(op)>(op) (
                vr, vi, vl);
        };
}

} // namespace detail
} // namespace immer
