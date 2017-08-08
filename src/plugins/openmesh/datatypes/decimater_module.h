#pragma once

#include <functional>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>

#include <possumwood_sdk/traits.h>

#include "openmesh.h"

class DecimaterModule {
	public:
		typedef std::function<void(OpenMesh::Decimater::DecimaterT<Mesh>&)> DecFn;

		DecimaterModule();
		DecimaterModule(const DecFn& fn);

		void operator() (OpenMesh::Decimater::DecimaterT<Mesh>& dec) const;

		bool operator == (const DecimaterModule& mod) const;
		bool operator != (const DecimaterModule& mod) const;

	protected:
	private:
		DecFn m_fn;
		std::size_t m_id;
};

namespace possumwood {

template<>
struct Traits<DecimaterModule> {
	static IO<DecimaterModule> io;
};

template<>
struct Traits<std::vector<DecimaterModule>> {
	static IO<std::vector<DecimaterModule>> io;
};

}
