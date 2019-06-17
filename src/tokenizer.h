#ifndef _TINYAP_TOKENIZER_H__
#define _TINYAP_TOKENIZER_H__

#include "lr_visitors.h"
#include <unordered_set>
#include <list>
#include <memory>

struct tokenizer {
    std::list<const grammar::item::base*> m_recognizers;
    std::unordered_map<const grammar::item::base*, std::pair<ast_node_t, unsigned int>> m_results;

    void deinit()
    {
        m_recognizers.clear();
        for (auto& kv: m_results) {
            if (kv.second.first) {
                unref(kv.second.first);
            }
        }
    }

    template <typename Iterator>
    void init(Iterator state_begin, Iterator state_end)
    {
        deinit();
        std::unordered_set<const grammar::item::base*> rec_set;
        for (auto i = state_begin; i != state_end; ++i) {
            for(const auto& t: (*i)->id.S->transitions.from_text_t) { rec_set.emplace(t.first); }
            for(const auto& t: (*i)->id.S->transitions.from_text_bow) { rec_set.emplace(t.first); }
            for(const auto& t: (*i)->id.S->transitions.from_text_re) { rec_set.emplace(t.first); }
        }

        for (const auto* i: rec_set) {
            m_recognizers.emplace_back(i);
        }
        std::cout << "[tokenizer] have " << m_recognizers.size() << " recognizers" << std::endl;
    }

    size_t consume(const char* source, unsigned int offset, unsigned int size)
    {
        decltype(m_results) tmp;

        unsigned int max_ofs = offset;

        std::clog << "[tokenizer] " << m_recognizers.size() << " recognizers on <<<" << std::string(source + offset, std::min(source + size, source + offset + 32)) << (size - offset > 32 ? "...>>>@" : ">>>@") << offset << '/' << size << std::endl;
        grammar::visitors::debugger debug(std::clog);
        for (const auto* i: m_recognizers) {
            std::clog << "[tokenizer] * " << i << std::endl;
            auto& ao = tmp[i] = i->recognize(source, offset, size);
            if (ao.first) {
                std::clog << "[tokenizer]    MATCH! " << ao.first << std::endl;
                ref(ao.first);
                if (ao.second > max_ofs) {
                    max_ofs = ao.second;
                }
            }
        }
        m_results.clear();
        for (const auto& kv: tmp) {
            if (kv.second.second == max_ofs) {
                m_results[kv.first] = kv.second;
            } else if (kv.second.first) {
                unref(kv.second.first);
            }
        }
        std::clog << "[tokenizer] " << m_results.size() << " recognizer(s) consumed " << max_ofs << " characters" << std::endl;

        return max_ofs;
    }

    bool consume(std::ostream& os)
    {
        // TODO scan character by character across all recognizers instead
        return false;
    }

    std::pair<ast_node_t, unsigned int>
    get_production_of(const grammar::item::base* i) const
    {
        auto it = m_results.find(i);
        if (it != m_results.end()) {
            return it->second;
        }
        return {NULL, 0};
    }
};


#endif

