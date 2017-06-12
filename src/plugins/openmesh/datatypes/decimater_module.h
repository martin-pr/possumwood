#pragma once

#include <functional>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>

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
