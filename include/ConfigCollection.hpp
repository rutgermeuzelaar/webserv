/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigCollection.hpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 17:41:54 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 19:09:22 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_COLLECTION_HPP
# define CONFIG_COLLECTION_HPP
# include <stdexcept>
# include <vector>

template<class T>
class ConfigCollection
{
    public:
        std::vector<T> m_vector;
        void add_unique(T obj)
        {
            for (const T& it: m_vector)
            {
                if (it == obj)
                {
                    throw std::runtime_error("Conflict with previously defined property of this type.");
                }
            }
            m_vector.push_back(obj);
        }
        void add(T obj)
        {
            m_vector.push_back(obj);
        }
};
#endif