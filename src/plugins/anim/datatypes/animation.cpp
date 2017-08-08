#include "animation.h"

namespace possumwood {

namespace {

void toJson(::dependency_graph::io::json& json, const std::shared_ptr<const anim::Animation>& value) {
}

void fromJson(const ::dependency_graph::io::json& json, std::shared_ptr<const anim::Animation>& value) {
}

}

IO<std::shared_ptr<const anim::Animation>> Traits<std::shared_ptr<const anim::Animation>>::io(&toJson, &fromJson);

}
