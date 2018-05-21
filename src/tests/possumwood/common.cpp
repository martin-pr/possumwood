#include "common.h"

#include <dependency_graph/attr.inl>
#include <dependency_graph/values.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/metadata_register.h>

const dependency_graph::MetadataHandle& additionNode() {
	static std::unique_ptr<dependency_graph::MetadataHandle> s_handle;

	if(s_handle == nullptr) {
		std::unique_ptr<dependency_graph::Metadata> meta(new dependency_graph::Metadata("addition"));

		// create attributes
		static dependency_graph::InAttr<float> additionInput1, additionInput2;
		static dependency_graph::OutAttr<float> additionOutput;

		// add attributes to the Metadata instance
		meta->addAttribute(additionInput1, "input_1");
		meta->addAttribute(additionInput2, "input_2");
		meta->addAttribute(additionOutput, "output");

		// setup influences
		meta->addInfluence(additionInput1, additionOutput);
		meta->addInfluence(additionInput2, additionOutput);

		std::vector<std::reference_wrapper<const dependency_graph::Attr>> influences;

		influences = meta->influences(additionInput1);

		std::function<dependency_graph::State(dependency_graph::Values&)> additionCompute = [&](dependency_graph::Values& data) {
			const float a = data.get(additionInput1);
			const float b = data.get(additionInput2);

			data.set(additionOutput, a + b);

			return dependency_graph::State();
		};
		meta->setCompute(additionCompute);

		s_handle = std::unique_ptr<dependency_graph::MetadataHandle>(new dependency_graph::MetadataHandle(std::move(meta)));

		dependency_graph::MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}

const dependency_graph::MetadataHandle& multiplicationNode() {
	static std::unique_ptr<dependency_graph::MetadataHandle> s_handle;

	if(s_handle == nullptr) {
		std::unique_ptr<dependency_graph::Metadata> meta(new dependency_graph::Metadata("multiplication"));

		static dependency_graph::InAttr<float> multiplicationInput1, multiplicationInput2;
		static dependency_graph::OutAttr<float> multiplicationOutput;
		std::function<dependency_graph::State(dependency_graph::Values&)> multiplicationCompute = [&](dependency_graph::Values & data) {
			const float a = data.get(multiplicationInput1);
			const float b = data.get(multiplicationInput2);

			data.set(multiplicationOutput, a * b);

			return dependency_graph::State();
		};

		meta->addAttribute(multiplicationInput1, "input_1");
		meta->addAttribute(multiplicationInput2, "input_2");
		meta->addAttribute(multiplicationOutput, "output");

		meta->addInfluence(multiplicationInput1, multiplicationOutput);
		meta->addInfluence(multiplicationInput2, multiplicationOutput);

		meta->setCompute(multiplicationCompute);

		s_handle = std::unique_ptr<dependency_graph::MetadataHandle>(new dependency_graph::MetadataHandle(std::move(meta)));

		dependency_graph::MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}
