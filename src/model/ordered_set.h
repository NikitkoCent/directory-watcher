#ifndef ORDERED_SET_H
#define ORDERED_SET_H

#include <vector>           // content field
#include <unordered_map>    // indices field
#include <algorithm>        // std::min
#include <utility>          // std::move, std::forward, etc.
#include <iterator>         // std::next, etc.


/* Insertion order preserving Set */
template<class Key>
class OrderedSet
{
private:
    template<bool condition, typename T1, typename T2>
    struct ConditionType
    {
        using type = T1;
    };

    template<typename T1, typename T2>
    struct ConditionType<false, T1, T2>
    {
        using type = T2;
    };

    using Content = std::vector<Key>;
    using Indices = std::unordered_map<Key, typename Content::size_type>;

    Content content;
    Indices indices;

public:
    using key_type = Key;
    using value_type = typename Content::value_type;
    using size_type = typename ConditionType<(sizeof(typename Content::size_type) < sizeof(typename Indices::size_type)),
                                             typename Content::size_type,
                                             typename Indices::size_type>::type;
    using difference_type = typename Content::difference_type;
    using reference = typename Content::reference;
    using const_reference = typename Content::const_reference;
    using pointer = typename Content::pointer;
    using const_pointer = typename Content::const_pointer;
    using iterator = typename Content::const_iterator;
    using const_iterator = typename Content::const_iterator;
    using reverse_iterator = typename Content::const_reverse_iterator;
    using const_reverse_iterator = typename Content::const_reverse_iterator;


    iterator begin() noexcept { return content.begin(); }
    const_iterator begin() const noexcept { return content.begin(); }
    const_iterator cbegin() const noexcept { return content.cbegin(); }

    iterator end() noexcept { return content.end(); }
    const_iterator end() const noexcept { return content.end(); }
    const_iterator cend() const noexcept { return content.cend(); }

    reverse_iterator rbegin() noexcept { return content.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return content.rbegin(); }
    const_reverse_iterator crbegin() const noexcept { return content.crbegin(); }

    reverse_iterator rend() noexcept { return content.rend(); }
    const_reverse_iterator rend() const noexcept { return content.rend(); }
    const_reverse_iterator crend() const noexcept { return content.crend(); }

    bool empty() const noexcept { return content.empty(); }
    size_type size() const noexcept { return content.size(); }
    size_type max_size() const noexcept
    {
        using ParamType = typename ConditionType<(sizeof(typename Content::size_type) > sizeof(typename Indices::size_type)),
                                                 typename Content::size_type,
                                                 typename Indices::size_type>::type;
        return std::min<ParamType>(content.max_size(), indices.max_size());
    }


    void clear() noexcept
    {
        content.clear();
        indices.clear();
    }


    iterator insert(const_iterator pos, const key_type &value)
    {
        auto insertResult = indices.insert({value, std::distance(begin(), pos)});
        if (insertResult.second)
        {
            try
            {
                const auto result = content.insert(pos, value);
                updateIndices(std::next(result), 1);
                return result;
            }
            catch (...)
            {
                indices.erase(insertResult.first);
                throw;
            }
        }

        return end();
    }

    iterator insert(const_iterator pos, key_type &&value)
    {
        auto insertResult = indices.insert({std::move(value), std::distance(begin(), pos)});
        if (insertResult.second)
        {
            try
            {
                const auto result = content.insert(pos, insertResult.first->first);
                updateIndices(std::next(result), 1);
                return result;
            }
            catch (...)
            {
                indices.erase(insertResult.first);
                throw;
            }
        }

        return end();
    }


    iterator push_front(const key_type &value)
    {
        return insert(cbegin(), value);
    }

    iterator push_front(key_type &&value)
    {
        return insert(cbegin(), std::move(value));
    }


    iterator push_back(const key_type &value)
    {
        return insert(cend(), value);
    }

    iterator push_back(key_type &&value)
    {
        return insert(cend(), std::move(value));
    }


    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        auto insertResult = indices.emplace(std::forward<Args>(args)..., std::distance(begin(), pos));
        if (insertResult.second)
        {
            try
            {
                const auto result = content.insert(pos, insertResult.first->first);
                updateIndices(std::next(result), 1);
                return result;
            }
            catch (...)
            {
                indices.erase(insertResult.first);
                throw;
            }
        }

        return end();
    }


    template<typename... Args>
    iterator emplace_front(Args&&... args)
    {
        return emplace(cbegin(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    iterator emplace_back(Args&&... args)
    {
        return emplace(cend(), std::forward<Args>(args)...);
    }


    iterator find(const key_type &value)
    {
         const auto result = indices.find(value);
         if (result != indices.end())
         {
             return std::next(begin(), result->second);
         }

         return end();
    }

    const_iterator find(const key_type &value) const
    {
        const auto result = indices.find(value);
        if (result != indices.cend())
        {
            return std::next(cbegin(), result->second);
        }

        return cend();
    }


    iterator erase(const_iterator pos)
    {
        return eraseImpl(indices.find(*pos));
    }

    size_type erase(const key_type &value)
    {
        const auto findResult = indices.find(value);
        if (findResult != indices.end())
        {
            eraseImpl(findResult);
            return 1;
        }

        return 0;
    }


    void pop_front()
    {
        erase(cbegin());
    }


    void pop_back()
    {
        erase(--cend());
    }


    bool assignElement(const_iterator pos, const key_type &newValue)
    {
        return assignElementImpl(pos, newValue);
    }

    bool assignElement(const_iterator pos, key_type &&newValue)
    {
        return assignElementImpl(pos, std::move(newValue));
    }

private:
    void updateIndices(typename Content::const_iterator start, typename Content::difference_type offset)
    {
        for (; start != content.cend(); ++start)
        {
            indices[*start] += offset;
        }
    }

    iterator eraseImpl(typename Indices::const_iterator indicesIterator)
    {
        const auto pos = indicesIterator->second;        
        indices.erase(indicesIterator);

        const auto result = content.erase(std::next(content.cbegin(), pos));
        updateIndices(result, -1);

        return result;
    }

    template<typename K>
    bool assignElementImpl(const_iterator pos, K &&newValue)
    {
        const auto iter = indices.find(*pos);
        if (iter != indices.end())
        {
            const auto pos = iter->second;
            content[pos] = std::forward<K>(newValue);
            indices.erase(iter);
            indices.insert({newValue, pos});

            return true;
        }

        return false;
    }
};

#endif // ORDERED_SET_H
