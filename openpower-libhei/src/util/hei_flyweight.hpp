#pragma once

#include <stdint.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace libhei
{

/** @brief A generic flyweight factory for objects of type T. */
template <class T>
class Flyweight
{
  private: // This class cannot be instantiated. Use getSingleton() instead.
    /** @brief Default constructor. */
    Flyweight() = default;

    /** @brief Destructor. */
    ~Flyweight() = default;

    /** @brief Default copy constructor. */
    Flyweight(const Flyweight&) = delete;

    /** @brief Default assignment operator. */
    Flyweight& operator=(const Flyweight&) = delete;

  public:
    /** @brief Provides access to a singleton instance of this object. */
    static Flyweight& getSingleton()
    {
        static Flyweight theFlyweight;
        return theFlyweight;
    }

    /**
     * @brief  Does an emplace add of a new entry to the factory, if the entry
     *         does not already exist, and returns a pointer to the entry in the
     *         factory.
     * @param  An variable argument list that would be passed to a contructor of
     *         class T.
     * @return A pointer to this entry in the factory.
     */
    template <class... Args>
    std::shared_ptr<T> get(Args&&... i_args)
    {
        // Create a new instance with the given arguments.
        std::shared_ptr<T> newEntry = std::make_shared<T>(i_args...);

        // The index is sorted by the value of the T objects. Check to see if
        // newEntry already exists in the factory.
        auto itr = std::lower_bound(
            iv_index.begin(), iv_index.end(), newEntry,
            [](const std::shared_ptr<T> a, const std::shared_ptr<T> b) {
                return *a < *b;
            });

        // std::lower_bound() will return the first element that does not
        // compare less than newEntry. So if an element is found, we must make
        // sure it does not have the same value as newEntry.
        if (iv_index.end() == itr || !(*newEntry == *(*itr)))
        {
            // Store the new enty in the sorted index.
            itr = iv_index.insert(itr, newEntry);
        }

        // It is important to note that if newEntry was not inserted into the
        // index above because a duplicate already existed, it will be deleted
        // as soon as it goes out of scope.

        // Return a pointer to the this entry in the factory.
        return *itr;
    }

    /**
     * @brief Deletes all entries in the factory.
     *
     * This is called in the destructor. So it cannot throw an exception.
     */
    void clear()
    {
        iv_index.clear();
    }

    /**
     * @brief Shrinks the index capacity to eliminate unused memory space.
     *
     * The index is a vector where the capacity will grow as items are added to
     * it. Each time more capacity is needed, the vector will double the current
     * capacity to make room for new entries. The general use of this class is
     * expected that all needed entries will be added during the isolator
     * initialization. Afterwards, the extra capacity is not needed. So this
     * function will shrink the capacity to the size of the vector.
     */
    void compact()
    {
        iv_index.shrink_to_fit();
    }

    /** @return The number of entries in this flyweight factory. */
    size_t size()
    {
        return iv_index.size();
    }

  private:
    /**
     * Each new T is allocated on the memory heap and a pointer to each of those
     * objects is stored in this vector. The vector will not contain duplicates
     * and is kept sorted by the value of the objects. Inserting a new element
     * is O(n). However, since the vector is sorted, searching for existing
     * entries is O(log n). Considering the expected use of this class,
     * insertion should only exist during initialization of the isolator, where
     * searching will be done at the time of isolation when performance is more
     * important.
     *
     * An alternative to this approach is to use std::set, where insertion and
     * searching are both O(log n). However, std::set is typically designed with
     * a red-black tree which carries a lot of extra memory overhead to maintain
     * the structure. Also, the Hostboot user application does not support
     * std::set at this time.
     */
    std::vector<std::shared_ptr<T>> iv_index;
};

} // end namespace libhei
