#ifndef __OY_SPLAYARRAY__
#define __OY_SPLAYARRAY__

#include <functional>
#include "MemoryPool.h"

namespace OY {
    template <typename _Tp>
    class SplayArray {
        struct node : MemoryPool<node> {
            _Tp key;
            bool reversed;
            int subtree_weight;
            node *lchild;
            node *rchild;
            node() = default;
            void reverse() { reversed = reversed ? false : true; }
            void push_down() {
                reverse();
                std::swap(lchild, rchild);
                if (lchild) lchild->reverse();
                if (rchild) rchild->reverse();
            }
        };
        node *m_root;
        int m_state;
        static int subtree_weight(node *p) { return p ? p->subtree_weight : 0; }
        static node *update(node *p) {
            if (p) p->subtree_weight = 1 + subtree_weight(p->lchild) + subtree_weight(p->rchild);
            return p;
        }
        static node *rrotate(node *p) {
            node *q = p->lchild;
            p->lchild = q->rchild;
            q->rchild = update(p);
            return q;
        }
        static node *lrotate(node *p) {
            node *q = p->rchild;
            p->rchild = q->lchild;
            q->lchild = update(p);
            return q;
        }
        static node *rrrotate(node *p) {
            node *q = p->lchild;
            node *s = q->lchild;
            p->lchild = q->rchild;
            q->rchild = update(p);
            q->lchild = s->rchild;
            s->rchild = update(q);
            return s;
        }
        static node *llrotate(node *p) {
            node *q = p->rchild;
            node *s = q->rchild;
            p->rchild = q->lchild;
            q->lchild = update(p);
            q->rchild = s->lchild;
            s->lchild = update(q);
            return s;
        }
        static node *lrrotate(node *p) {
            node *q = p->lchild;
            node *s = q->rchild;
            p->lchild = s->rchild;
            s->rchild = update(p);
            q->rchild = s->lchild;
            s->lchild = update(q);
            return s;
        }
        static node *rlrotate(node *p) {
            node *q = p->rchild;
            node *s = q->lchild;
            p->rchild = s->lchild;
            s->lchild = update(p);
            q->lchild = s->rchild;
            s->rchild = update(q);
            return s;
        }
        node *update_from_lchild(node *cur, node *root) {
            m_state = m_state * 2 + 1;
            if (m_state == 3) {
                m_state = 0;
                return rrrotate(cur);
            } else if (m_state == 5) {
                m_state = 0;
                return lrrotate(cur);
            } else if (m_state == 1 && cur == root)
                return rrotate(cur);
            else
                return cur;
        }
        node *update_from_rchild(node *cur, node *root) {
            m_state = m_state * 2 + 2;
            if (m_state == 4) {
                m_state = 0;
                return rlrotate(cur);
            } else if (m_state == 6) {
                m_state = 0;
                return llrotate(cur);
            } else if (m_state == 2 && cur == root)
                return lrotate(cur);
            else
                return cur;
        }
        node *splay_max(node *cur, node *const &root) {
            if (cur->reversed) cur->push_down();
            if (cur->rchild) {
                cur->rchild = splay_max(cur->rchild, root);
                if (++m_state == 2) {
                    m_state = 0;
                    return llrotate(cur);
                } else if (m_state == 1 && cur == root)
                    return lrotate(cur);
            } else
                m_state = 0;
            return cur;
        }
        node *splay_min(node *cur, node *const &root) {
            if (cur->reversed) cur->push_down();
            if (cur->lchild) {
                cur->lchild = splay_min(cur->lchild, root);
                if (++m_state == 2) {
                    m_state = 0;
                    return rrrotate(cur);
                } else if (m_state == 1 && cur == root)
                    return rrotate(cur);
            } else
                m_state = 0;
            return cur;
        }
        node *splay_kth(node *cur, node *const &root, int k) {
            if (cur->reversed) cur->push_down();
            if (k < subtree_weight(cur->lchild)) {
                cur->lchild = splay_kth(cur->lchild, root, k);
                return update_from_lchild(cur, root);
            } else if (k -= subtree_weight(cur->lchild); k) {
                cur->rchild = splay_kth(cur->rchild, root, k - 1);
                return update_from_rchild(cur, root);
            } else {
                m_state = 0;
                return cur;
            }
        }
        template <typename _Iterator>
        node *make_tree(_Iterator first, _Iterator last) {
            auto _make_tree = [](auto self, _Iterator first, _Iterator last) -> node * {
                if (first == last) return nullptr;
                if (first + 1 == last) return new node{{}, *first, false, 1, nullptr, nullptr};
                _Iterator mid = first + (last - first) / 2;
                node *p = new node{{}, *mid, false, 1, self(self, first, mid), self(self, mid + 1, last)};
                return update(p);
            };
            return _make_tree(_make_tree, first, last);
        }
        void _clear(node *cur) {
            // if (cur->lchild) _clear(cur->lchild);
            // if (cur->rchild) _clear(cur->rchild);
            // delete cur;
        }

    public:
        static void setBufferSize(int __count) { MemoryPool<node>::_reserve(__count); }
        SplayArray() : m_root(nullptr) {}
        template <typename _Iterator>
        SplayArray(_Iterator __first, _Iterator __last) : m_root(make_tree(__first, __last)) {}
        void clear() {
            if (m_root) _clear(m_root);
            m_root = nullptr;
        }
        void insert(int __pos, _Tp __key) {
            if (!__pos)
                push_front(__key);
            else if (__pos == size())
                push_back(__key);
            else {
                m_root = splay_kth(m_root, m_root, __pos);
                node *p = new node{{}, __key, false, 1, m_root->lchild, nullptr};
                m_root->lchild = nullptr;
                p->rchild = update(m_root);
                m_root = update(p);
            }
        }
        void insert(int __pos, SplayArray<_Tp> &__toInsert) {
            if (__pos == size())
                join(__toInsert);
            else {
                SplayArray<_Tp> sub = subArray(__pos, size() - 1);
                join(__toInsert);
                join(sub);
            }
        }
        template <typename _Iterator>
        void insert(int __pos, _Iterator __first, _Iterator __last) {
            if (SplayArray<_Tp> toInsert(__first, __last); toInsert.size()) insert(__pos, toInsert);
        }
        void update(int __pos, _Tp __key) { at(__pos)->key = __key; }
        void erase(int __pos) {
            if (!__pos)
                pop_front();
            else if (__pos == size() - 1)
                pop_back();
            else {
                m_root = splay_kth(m_root, m_root, __pos);
                node *lchild = m_root->lchild = splay_max(m_root->lchild, m_root->lchild);
                lchild->rchild = m_root->rchild;
                delete m_root;
                m_root = update(lchild);
            }
        }
        void erase(int __left, int __right) {
            if (!__left) {
                node *p = splay_kth(m_root, m_root, __right);
                m_root = p->rchild;
                p->rchild = nullptr;
                _clear(p);
            } else if (__right == size() - 1) {
                node *p = splay_kth(m_root, m_root, __left);
                m_root = p->lchild;
                p->lchild = nullptr;
                _clear(p);
            } else {
                node *p = splay_kth(m_root, m_root, __right + 1);
                node *q = splay_kth(p->lchild, p->lchild, __left);
                p->lchild = q->lchild;
                m_root = update(p);
                q->lchild = nullptr;
                _clear(q);
            }
        }
        template <typename _Iterator>
        void assign(_Iterator __first, _Iterator __last) {
            clear();
            m_root = make_tree(__first, __last);
        }
        void reverse(int __left, int __right) {
            if (!__left) {
                if (__right == size() - 1)
                    m_root->reverse();
                else {
                    m_root = update(splay_kth(m_root, m_root, __right + 1));
                    m_root->lchild->reverse();
                }
            } else if (__right == size() - 1) {
                m_root = update(splay_kth(m_root, m_root, __left - 1));
                m_root->rchild->reverse();
            } else {
                m_root = update(splay_kth(m_root, m_root, __right + 1));
                m_root->lchild = update(splay_kth(m_root->lchild, m_root->lchild, __left - 1));
                m_root->lchild->rchild->reverse();
            }
        }
        void push_front(_Tp __key) {
            if (!m_root)
                m_root = new node{{}, __key, false, 1, nullptr, nullptr};
            else {
                m_root = splay_min(m_root, m_root);
                m_root->lchild = new node{{}, __key, false, 1, nullptr, nullptr};
                m_root = update(m_root);
            }
        }
        void pop_front() {
            m_root = splay_min(m_root, m_root);
            node *rchild = m_root->rchild;
            delete m_root;
            m_root = rchild;
        }
        void push_back(_Tp __key) {
            if (!m_root)
                m_root = new node{{}, __key, false, 1, nullptr, nullptr};
            else {
                m_root = splay_max(m_root, m_root);
                m_root->rchild = new node{{}, __key, false, 1, nullptr, nullptr};
                m_root = update(m_root);
            }
        }
        void pop_back() {
            m_root = splay_max(m_root, m_root);
            node *lchild = m_root->lchild;
            delete m_root;
            m_root = lchild;
        }
        node *at(int __pos) { return m_root = update(splay_kth(m_root, m_root, __pos)); }
        _Tp &operator[](int __index) { return at(__index)->key; }
        node *front() { return m_root = update(splay_min(m_root, m_root)); }
        node *back() { return m_root = update(splay_max(m_root, m_root)); }
        int size() const { return subtree_weight(m_root); }
        bool empty() const { return !size(); }
        SplayArray<_Tp> subArray(int __left, int __right) {
            SplayArray<_Tp> sub;
            if (!__left) {
                node *p = splay_kth(m_root, m_root, __right);
                m_root = p->rchild;
                p->rchild = nullptr;
                sub.m_root = update(p);
            } else if (__right == size() - 1) {
                node *p = splay_kth(m_root, m_root, __left);
                m_root = p->lchild;
                p->lchild = nullptr;
                sub.m_root = update(p);
            } else {
                node *p = splay_kth(m_root, m_root, __right + 1);
                node *q = splay_kth(p->lchild, p->lchild, __left);
                p->lchild = q->lchild;
                m_root = update(p);
                q->lchild = nullptr;
                sub.m_root = update(q);
            }
            return sub;
        }
        void join(SplayArray<_Tp> &__other) {
            if (!m_root)
                m_root = __other.m_root;
            else {
                m_root = splay_max(m_root, m_root);
                m_root->rchild = __other.m_root;
            }
            __other.m_root = nullptr;
            m_root = update(m_root);
        }
        template <typename _Sequence = std::vector<_Tp>>
        _Sequence to_sequence() const {
            _Sequence v;
            v.reserve(size());
            auto dfs = [&](auto self, node *cur) -> void {
                if (cur->reversed) cur->push_down();
                if (cur->lchild) self(self, cur->lchild);
                v.push_back(cur->key);
                if (cur->rchild) self(self, cur->rchild);
            };
            if (m_root) dfs(dfs, m_root);
            return v;
        }
        template <typename _Sequence = std::vector<_Tp>>
        _Sequence to_sequence(int __left, int __right) {
            _Sequence v;
            v.reserve(__right - __left + 1);
            auto dfs = [&](auto self, node *cur) -> void {
                if (cur->reversed) cur->push_down();
                if (cur->lchild) self(self, cur->lchild);
                if (v.size() == __right - __left + 1) return;
                v.push_back(cur->key);
                if (v.size() == __right - __left + 1) return;
                if (cur->rchild) self(self, cur->rchild);
            };
            m_root = update(splay_kth(m_root, m_root, __left));
            v.push_back(m_root->key);
            if (__right > __left) dfs(dfs, m_root->rchild);
            return v;
        }
    };
    template <typename _Tp = int>
    SplayArray() -> SplayArray<_Tp>;
    template <typename _Iterator, typename _Tp = typename std::iterator_traits<_Iterator>::value_type>
    SplayArray(_Iterator, _Iterator) -> SplayArray<_Tp>;
}

#endif