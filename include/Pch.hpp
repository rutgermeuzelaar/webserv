#ifndef PCH_HPP
# define PCH_HPP
// cat src/*/*.cpp | grep "include <" | sort | uniq -c | sort -nr
// cat include/*.hpp | grep "include <" | sort | uniq -c | sort -nr
// std
# ifdef NDEBUG
// data structures
#  include <string>
#  include <vector>
#  include <unordered_map>
#  include <map>
#  include <optional>
#  include <tuple>
#  include <memory>

#  include <iostream>
#  include <algorithm>
#  include <stdexcept>
#  include <cassert>
#  include <utility>
#  include <filesystem>
#  include <cstddef>
# endif
#endif