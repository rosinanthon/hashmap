#include <iostream>
#include <vector>
#include <exception>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    const static size_t load_factor_coefficient = 2;
    struct Elem {
        int psl;
        std::pair<const KeyType, ValueType> *pr;

        Elem() {
            psl = -1;
            pr = nullptr;
        }

        Elem &operator=(Elem &&other) noexcept {
            psl = other.psl;
            pr = other.pr;
            other.pr = nullptr;
            return *this;
        }

        Elem(const Elem &other) {
            psl = other.psl;
            pr = other.pr;
        }

        Elem &operator=(const Elem &other) {
            psl = other.psl;
            pr = other.pr;
            return *this;
        }

        Elem(int psl_, std::pair<const KeyType, ValueType> *pr_) {
            psl = psl_;
            pr = pr_;
        }

        friend void swap(Elem &e1, Elem &e2) {
            Elem e3;
            e3 = std::move(e1);
            e1 = std::move(e2);
            e2 = std::move(e3);
        }

        ~Elem() {
            if (pr != nullptr) {
                delete pr;
            }
        }

        bool operator==(const Elem &e2) const {
            return e2.pr == pr && e2.psl == psl;
        }

        const KeyType &key() const {
            return pr->first;
        }

        ValueType &val() {
            return pr->second;
        }

        ValueType &val() const {
            return pr->second;
        }
    };

    std::vector<Elem> data;
    Hash hasher;
    size_t cnt;
public:
    HashMap() : data({Elem()}), hasher(Hash()), cnt(0) {};

    HashMap(Hash h) : data({Elem()}), hasher(h), cnt(0) {};

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &l) {
        data = {Elem()};
        hasher = Hash();
        cnt = 0;
        for (auto i: l) {
            insert(i);
        }
    }

    HashMap(const HashMap &v) {
        if (v.data == data) {
            hasher = v.hasher;
            return;
        }
        std::vector<Elem>().swap(data);
        data.resize(v.mem());
        hasher = v.hasher;
        cnt = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            if (v.data[i].psl != -1) {
                insert(*v.data[i].pr);
            }
        }
    }

    HashMap &operator=(const HashMap &v) {
        if (v.data == data) {
            hasher = v.hasher;
            return *this;
        }
        std::vector<Elem>().swap(data);
        data.resize(v.mem());
        hasher = v.hasher;
        cnt = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            if (v.data[i].psl != -1) {
                insert(*v.data[i].pr);
            }
        }
        return *this;
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> l, Hash h) {
        data = {Elem()};
        hasher = h;
        cnt = 0;
        for (auto i: l) {
            insert(i);
        }
    }

    template<typename Iterator>
    HashMap(Iterator begin, Iterator end) {
        data = {Elem()};
        hasher = Hash();
        cnt = 0;
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

    template<typename Iterator>
    HashMap(Iterator begin, Iterator end, Hash h) {
        data = {Elem()};
        hasher = h;
        cnt = 0;
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

private:
    size_t getpos(const KeyType& key) const {
        size_t curr = hasher(key) % data.size();
        for (size_t i = 0; i < data.size(); ++i, curr = (curr + 1 == data.size() ? 0 : curr + 1)) {
            if (data[curr].psl == -1) {
                return data.size();
            }
            if (data[curr].key() == key) {
                return curr;
            }
        }
        return data.size();
    }

    void insert_elem(Elem &e) {
        size_t pos = getpos(e.key());
        if (pos != data.size()) {
            return;
        }
        size_t curr = hasher(e.key()) % data.size();
        for (size_t i = 0; i < data.size(); ++i, curr = (curr + 1 == data.size() ? 0 : curr + 1)) {
            if (data[curr].psl == -1) {
                swap(e, data[curr]);
                break;
            }
            if (data[curr].psl < e.psl) {
                swap(e, data[curr]);
            }
            ++e.psl;
        }
        ++cnt;
        resize(true);
    }

public:
    void insert(std::pair<KeyType, ValueType> v) {
        Elem e;
        e.psl = 0;
        e.pr = new std::pair<const KeyType, ValueType>(v);
        insert_elem(e);
    }

    void erase(KeyType v) {
        size_t pos = getpos(v);
        if (pos != data.size()) {
            delete data[pos].pr;
            data[pos] = Elem();
            --cnt;
            for (size_t i = 0; i < data.size(); ++i) {
                size_t nxt = pos + 1;
                if (nxt == data.size()) {
                    nxt = 0;
                }
                if (data[nxt].psl < 1) {
                    break;
                }
                swap(data[nxt], data[pos]);
                --data[pos].psl;
                pos = nxt;
            }
        }
        resize(false);
    }

    ValueType &operator[](KeyType a) {
        size_t pos = getpos(a);
        if (pos == data.size()) {
            insert(std::pair<const KeyType, ValueType>{a, ValueType{}});
            pos = getpos(a);
        }
        return data[pos].val();
    }

    ValueType const &at(KeyType a) const {

        size_t pos = getpos(a);
        if (pos == data.size()) {
            throw std::out_of_range("");
        }
        return data[pos].val();
    }

    void clear() {
        std::vector<Elem>().swap(data);
        data = {Elem()};
        cnt = 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    size_t size() const {
        return cnt;
    }

    bool empty() const {
        return cnt == 0;
    }

    size_t mem() const {
        return data.size();
    }

    class iterator {
        HashMap<KeyType, ValueType, Hash> *hm;
    public:
        size_t pos;

        iterator() : hm(nullptr), pos(0) {};

        iterator(size_t pos_, HashMap<KeyType, ValueType, Hash> *hm_) : hm(hm_), pos(pos_) {};


        bool operator==(iterator it2) {
            return it2.pos == pos;
        }

        bool operator!=(iterator it2) {
            return it2.pos != pos;
        }

        iterator &operator++() {
            if (pos == hm->mem()) {
                return *this;
            }
            ++pos;
            while (pos < hm->mem() && hm->data[pos].psl == -1) {
                ++pos;
            }
            return *this;
        }

        iterator &operator--() {
            size_t pos_ = pos;
            if (!pos) {
                return *this;
            }
            --pos_;
            while (pos_ > 0 && hm->data[pos_].psl == -1) {
                --pos_;
            }
            if (hm->data[pos_].psl > -1) {
                pos = pos_;
            }
            return *this;
        }

        iterator operator++(int) {
            auto copy = *this;
            if (pos == hm->mem()) {
                return copy;
            }
            ++pos;
            while (pos < hm->mem() && hm->data[pos].psl == -1) {
                ++pos;
            }
            return copy;
        }

        iterator operator--(int) {
            auto copy = *this;
            size_t pos_ = pos;
            if (!pos) {
                return copy;
            }
            --pos_;
            while (pos_ > 0 && hm->data[pos_].psl == -1) {
                --pos_;
            }
            if (hm->data[pos_].psl > -1) {
                pos = pos_;
            }
            return copy;
        }

        std::pair<const KeyType, ValueType> &operator*() {
            return *hm->data[pos].pr;
        }

        std::pair<const KeyType, ValueType> *operator->() {
            return hm->data[pos].pr;
        }
    };

    iterator begin() {
        HashMap<KeyType, ValueType, Hash>::iterator it(0, this);
        if (data[0].psl != -1) {
            return it;
        }
        return ++it;
    }

    iterator end() {
        HashMap<KeyType, ValueType, Hash>::iterator it(data.size(), this);
        return it;
    }

    iterator find(KeyType key) {
        size_t pos = getpos(key);
        if (pos == mem()) {
            return end();
        }
        return HashMap<KeyType, ValueType, Hash>::iterator(pos, this);
    }


    class const_iterator {
        HashMap<KeyType, ValueType, Hash> const *hm;
    public:
        size_t pos;

        const_iterator() : hm(nullptr), pos(0) {};

        const_iterator(size_t pos_, HashMap<KeyType, ValueType, Hash> const *hm_) : hm(hm_), pos(pos_) {};

        const_iterator(const_iterator const &it) : hm(it.hm), pos(it.pos) {};

        bool operator==(const_iterator it2) const {
            return it2.pos == pos;
        }

        bool operator!=(const_iterator it2) const {
            return it2.pos != pos;
        }

        const_iterator &operator++() {
            if (pos == hm->mem()) {
                return *this;
            }
            ++pos;
            while (pos < hm->mem() && hm->data[pos].psl == -1) {
                ++pos;
            }
            return *this;
        }

        const_iterator &operator--() {
            size_t pos_ = pos;
            if (!pos) {
                return *this;
            }
            --pos_;
            while (pos_ > 0 && hm->data[pos_].psl == -1) {
                --pos_;
            }
            if (hm->data[pos_].psl > -1) {
                pos = pos_;
            }
            return *this;
        }

        const_iterator operator++(int) {
            auto copy = *this;
            if (pos == hm->mem()) {
                return copy;
            }
            ++pos;
            while (pos < hm->mem() && hm->data[pos].psl == -1) {
                ++pos;
            }
            return copy;
        }

        const_iterator operator--(int) {
            auto copy = *this;
            size_t pos_ = pos;
            if (!pos) {
                return copy;
            }
            --pos_;
            while (pos_ > 0 && hm->data[pos_].psl == -1) {
                --pos_;
            }
            if (hm->data[pos_].psl > -1) {
                pos = pos_;
            }
            return copy;
        }

        const std::pair<const KeyType, ValueType> &operator*() const {
            return *hm->data[pos].pr;
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return hm->data[pos].pr;
        }
    };

    const_iterator begin() const {
        HashMap<KeyType, ValueType, Hash>::const_iterator it(0, this);
        if (data[0].psl != -1) {
            return it;
        }
        return ++it;
    }

    const_iterator end() const {
        HashMap<KeyType, ValueType, Hash>::const_iterator it(data.size(), this);
        return it;
    }

    const_iterator find(KeyType key) const {
        size_t pos = getpos(key);
        if (pos == mem()) {
            return end();
        }
        return HashMap<KeyType, ValueType, Hash>::const_iterator(pos, this);
    }

private:
    void fill(std::vector<Elem> &val) {
        cnt = 0;
        for (size_t i = 0; i < val.size(); ++i) {
            if (val[i].psl > -1) {
                val[i].psl = 0;
                insert_elem(val[i]);
            }
        }
    }

    void resize(bool is_insert) {
        std::vector<Elem> old_data;
        size_t sz = data.size();
        while (is_insert && (sz < cnt * load_factor_coefficient)) {
            sz *= load_factor_coefficient;
        }
        while ((sz >= load_factor_coefficient) && !is_insert && (sz > cnt * load_factor_coefficient * load_factor_coefficient * load_factor_coefficient)) {
            sz /= load_factor_coefficient;
        }
        if (sz == data.size()) {
            return;
        }
        old_data.resize(sz);
        std::swap(data, old_data);
        fill(old_data);
        std::vector<Elem>().swap(old_data);
    }

};
