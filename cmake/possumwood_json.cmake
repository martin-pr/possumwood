find_package(nlohmann_json)

if(nlohmann_json_FOUND)
    set(JSON_INCLUDE_DIRS "")
    set(JSON_LIBS nlohmann_json::nlohmann_json)
else(nlohmann_json_FOUND)
    find_path(JSON_INCLUDE_DIRS NAMES nlohmann/json.hpp REQUIRED)
    set(JSON_LIBS "")
endif(nlohmann_json_FOUND)
