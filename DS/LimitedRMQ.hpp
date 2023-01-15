#ifndef __OY_LIMITEDRMQ__
#define __OY_LIMITEDRMQ__

#include "STtable.h"

namespace OY {
    template <typename _Sequence, typename _Compare = std::less<void>>
    struct _LimitedRMQPicker {
        _Sequence &m_seq;
        _Compare m_comp;
        _LimitedRMQPicker(_Sequence &__seq, _Compare __comp = _Compare()) : m_seq(__seq), m_comp(__comp) {}
        uint32_t operator()(uint32_t __x, uint32_t __y) const { return m_comp(m_seq[__x], m_seq[__y]) ? __y : __x; }
    };
    template <typename _Pick>
    class LimitedRMQ {
        _Pick m_pick;
        int m_length;
        int m_blockSize;
        int m_blockCount;
        std::vector<int> m_state;
        std::vector<std::vector<std::vector<int>>> m_internal;
        STTable<int, _Pick> m_table;

    public:
        LimitedRMQ(_Pick __pick = _Pick()) : m_pick(__pick), m_table(0, __pick) {}
        template <typename _Iterator>
        LimitedRMQ(_Iterator __first, _Iterator __last, _Pick __pick = _Pick()) : m_pick(__pick), m_table(0, __pick) { reset(__first, __last); }
        template <typename _Iterator>
        void reset(_Iterator __first, _Iterator __last) {
            m_length = __last - __first;
            m_blockSize = m_length > 1 ? (32 - std::__countl_zero<uint32_t>(m_length - 1)) / 2 : 1;
            m_blockCount = (m_length + m_blockSize - 1) / m_blockSize;
            m_state.clear();
            m_state.reserve(m_blockCount);
            m_internal.assign(1 << m_blockSize, std::vector<std::vector<int>>());
            int cursor = 0;
            while (cursor <= m_length - m_blockSize) {
                int state = 0;
                for (int i = 1; i < m_blockSize; i++) state = state * 2 + (__first[cursor + i] > __first[cursor + i - 1]);
                m_state.push_back(state);
                if (m_internal[state].empty()) {
                    m_internal[state].resize(m_blockSize, std::vector<int>(m_blockSize, 0));
                    for (int i = 0; i < m_blockSize; i++) {
                        int pos = cursor + i;
                        m_internal[state][i][i] = i;
                        for (int j = i + 1; j < m_blockSize; j++) {
                            pos = m_pick(pos, cursor + j);
                            m_internal[state][i][j] = pos - cursor;
                        }
                    }
                }
                cursor += m_blockSize;
            }
            if (cursor < m_length) {
                int state = 0;
                for (int i = 1; i < m_blockSize; i++) state = state * 2 + (cursor + i < m_length && __first[cursor + i] > __first[cursor + i - 1]);
                m_state.push_back(state);
                if (m_internal[state].empty()) {
                    m_internal[state].resize(m_blockSize, std::vector<int>(m_blockSize, 0));
                    for (int i = 0; i < m_blockSize && cursor + i < m_length; i++) {
                        int pos = cursor + i;
                        m_internal[state][i][i] = 0;
                        for (int j = i + 1; j < m_blockSize && cursor + j < m_length; j++) {
                            pos = m_pick(pos, cursor + j);
                            m_internal[state][i][j] = pos - cursor;
                        }
                    }
                }
            }
            if (int fullBlock = m_blockCount - (m_length % m_blockSize != 0); fullBlock > 1) {
                int blockValue[fullBlock];
                for (int i = 0; i < fullBlock; i++) blockValue[i] = i * m_blockSize + m_internal[m_state[i]].front().back();
                m_table.reset(blockValue, blockValue + fullBlock);
            }
        }
        int query(int __left, int __right) const {
            int l1 = __left / m_blockSize, l2 = __left % m_blockSize;
            int r1 = __right / m_blockSize, r2 = __right % m_blockSize;
            if (l1 == r1)
                return __left - l2 + m_internal[m_state[l1]][l2][r2];
            else if (l1 + 1 == r1)
                return m_pick(__left - l2 + m_internal[m_state[l1]][l2][m_blockSize - 1], __right - r2 + m_internal[m_state[r1]][0][r2]);
            else
                return m_pick(m_table.query(l1 + 1, r1 - 1), m_pick(__left - l2 + m_internal[m_state[l1]][l2][m_blockSize - 1], __right - r2 + m_internal[m_state[r1]][0][r2]));
        }
        int queryAll() const { return query(0, m_length - 1); }
    };
}

#endif