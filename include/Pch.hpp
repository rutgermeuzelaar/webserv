#ifndef PCH_HPP
# define PCH_HPP
// cat src/*/*.cpp | grep "include <" | sort | uniq -c | sort -nr
// cat include/*.hpp | grep "include <" | sort | uniq -c | sort -nr
// std
# ifndef NDEBUG
#  include <iostream>
#  include <stdexcept>
#  include <string>
#  include <algorithm>
#  include <vector>
#  include <cassert>

// maybe
#  include <filesystem>
# endif
#endif