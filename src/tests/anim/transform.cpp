#include <cmath>
#include <cstdlib>

#include <anim/datatypes/transform.h>

#include <ImathEuler.h>

#include <boost/test/unit_test.hpp>

using std::cout;
using std::endl;

float compareMatrices(const Imath::M44f& m1, const Imath::M44f& m2) {
	// cout << m1 << endl << m2 << endl << endl;

	float result = 0.0f;
	for(unsigned a=0;a<4;++a)
		for(unsigned b=0;b<4;++b)
			result += std::fabs(m1[a][b] - m2[a][b]);
	return result;
}

float compareMatrices(const Imath::M33f& m1, const Imath::M33f& m2) {
	// cout << m1 << endl << m2 << endl << endl;

	float result = 0.0f;
	for(unsigned a=0;a<3;++a)
		for(unsigned b=0;b<3;++b)
			result += std::fabs(m1[a][b] - m2[a][b]);
	return result;
}

float compareTransforms(const anim::Transform& m1, const anim::Transform& m2) {
	// cout << m1 << endl << m2 << endl << endl;

	// antipodality - two possible solutions to matrix-to-quat transformation - make sure we handle both
	float antipod1 = 0.0f, antipod2 = 0.0f;
	for(unsigned a=0;a<4;++a) {
		antipod1 += std::fabs(m1.rotation[a] - m2.rotation[a]);
		antipod2 += std::fabs(m1.rotation[a] + m2.rotation[a]);
	}
	float result = std::min(antipod1, antipod2);

	for(unsigned a=0;a<3;++a)
		result += std::fabs(m1.translation[a] - m2.translation[a]);

	return result;
}

namespace {
	static const float EPS = 1e-4f;
}

/////////////
// tests mainly the correspondence between the transformation class
// and a 4x4 matrix.

BOOST_AUTO_TEST_CASE(transform_init) {
	std::vector<std::pair<anim::Transform, Imath::M44f>> data = {
		{
			anim::Transform(),
			Imath::M44f(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)
		},
		{
			anim::Transform(Imath::V3f(1,2,3)),
			Imath::M44f(1,0,0,1, 0,1,0,2, 0,0,1,3, 0,0,0,1)
		},
		{
			anim::Transform(Imath::Quatf(-0.326096f, -0.0849528, -0.190674, 0.922001)),
			Imath::M44f(
				-0.772889 ,  0.633718, -0.0322979, 0,
				-0.568925 , -0.71461 , -0.407009 , 0,
				-0.28101  , -0.296198,  0.912853 , 0,
				0,0,0,1)
		},
		{
			anim::Transform(Imath::Quatf(-0.326096f, -0.0849528, -0.190674, 0.922001), Imath::V3f(3,4,5)),
			Imath::M44f(
				-0.772889 ,  0.633718, -0.0322979, 3,
				-0.568925 , -0.71461 , -0.407009 , 4,
				-0.28101  , -0.296198,  0.912853 , 5,
				0,0,0,1)
		}
	};

	// compare the resulting matrices (transform-to-matrix)
	for(auto& i : data)
		BOOST_REQUIRE_SMALL(
			compareMatrices(
				i.first.toMatrix44(),
				i.second
			),
			EPS
		);

	// and compare the transformation (matrix-to-transform)
	for(auto& i : data)
		BOOST_REQUIRE_SMALL(
			compareTransforms(
				i.first,
				anim::Transform(i.second)
			),
			EPS
		);

	// test inverses
	for(auto& i : data) {
		// cout << i.first.inverse().toMatrix44() << endl;
		// cout << i.second.inverse() << endl << endl;

		BOOST_REQUIRE_SMALL(
			compareTransforms(
				i.first.inverse(),
				i.second.inverse()
			),
			EPS
		);
	}

	// and test that multiplication with inverse always leads to unit
	for(auto& i : data) {
		BOOST_REQUIRE_SMALL(
			compareTransforms(
				i.first.inverse() * i.first,
				anim::Transform(Imath::Quatf(1,0,0,0))
			),
			EPS
		);

		BOOST_REQUIRE_SMALL(
			compareTransforms(
				i.first * i.first.inverse(),
				anim::Transform(Imath::Quatf(1,0,0,0))
			),
			EPS
		);
	}

	///
	{
		Imath::Quatf q;
		q.setAxisAngle(Imath::V3f(0,0,1), 0);

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				q.toMatrix33(),
				Imath::M33f(1,0,0, 0,1,0, 0,0,1)
			),
			EPS
		);
	}


	for(unsigned a=0;a<100;++a) {
		float angle = 8.0f * M_PI * (float)rand() / (float)RAND_MAX;

		Imath::Quatf q;
		q.setAxisAngle(Imath::V3f(1,0,0), angle);

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				q.toMatrix33(),
				Imath::M33f(1,0,0, 0,cos(angle),sin(angle), 0,-sin(angle),cos(angle))
			),
			EPS
		);
	}

	for(unsigned a=0;a<100;++a) {
		float angle = 8.0f * M_PI * (float)rand() / (float)RAND_MAX;

		Imath::Quatf q;
		q.setAxisAngle(Imath::V3f(0,1,0), angle);

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				q.toMatrix33(),
				Imath::M33f(cos(angle),0,-sin(angle), 0,1,0, sin(angle),0,cos(angle))
			),
			EPS
		);
	}

}

////////////////

namespace {
	const Imath::M44f toMatrix44(const std::pair<Imath::Eulerf, Imath::V3f>& data) {
		Imath::M44f result = data.first.toMatrix44().transpose();
		result[0][3] = data.second[0];
		result[1][3] = data.second[1];
		result[2][3] = data.second[2];

		return result;
	}
}

BOOST_AUTO_TEST_CASE(transform_operations) {
	std::vector<std::pair<
		std::pair<Imath::Eulerf, Imath::V3f>,
		std::pair<Imath::Eulerf, Imath::V3f>
	>> data = {
		{
			{
				Imath::Eulerf(-25,20,36),
				Imath::V3f(0,0,0)
			},
			{
				Imath::Eulerf(89,-93,63),
				Imath::V3f(0,0,0)
			}
		},
		{
			{
				Imath::Eulerf(-25,20,36),
				Imath::V3f(1,2,3)
			},
			{
				Imath::Eulerf(89,-93,63),
				Imath::V3f(0,0,0)
			}
		},
		{
			{
				Imath::Eulerf(-25,20,36),
				Imath::V3f(1,2,3)
			},
			{
				Imath::Eulerf(89,-93,63),
				Imath::V3f(7,6,5)
			}
		}
	};

	for(auto& i : data) {
		// cout << (anim::Transform(~i.first.first.toQuat(), i.first.second) * anim::Transform(~i.second.first.toQuat(), i.second.second)).toMatrix44() << endl;
		// cout << toMatrix44(i.first) * toMatrix44(i.second) << endl;

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				(anim::Transform(i.first.first.toQuat(), i.first.second) * anim::Transform(i.second.first.toQuat(), i.second.second)).toMatrix44(),
				toMatrix44(i.first) * toMatrix44(i.second)
			),
			EPS
		);
	}

	// cout << "-----" << endl;

	for(auto& i : data)
		BOOST_REQUIRE_SMALL(
			compareTransforms(
				anim::Transform(i.first.first.toQuat(), i.first.second) * anim::Transform(i.second.first.toQuat(), i.second.second),
				anim::Transform(toMatrix44(i.first) * toMatrix44(i.second))
			),
			EPS
		);

	for(auto& i : data) {
		anim::Transform t(i.first.first.toQuat(), i.first.second);
		t *= anim::Transform(i.second.first.toQuat(), i.second.second);

		// cout << t.toMatrix44() << endl;
		// cout << toMatrix44(i.first) * toMatrix44(i.second) << endl;

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				t.toMatrix44(),
				toMatrix44(i.first) * toMatrix44(i.second)
			),
			EPS
		);
	}
}

namespace {
	anim::Transform randomTransform() {
		const float a = (float)rand() / (float)RAND_MAX * 180.0f - 90.0f;
		const float b = (float)rand() / (float)RAND_MAX * 180.0f - 90.0f;
		const float c = (float)rand() / (float)RAND_MAX * 180.0f - 90.0f;
		Imath::Eulerf angle(a,b,c);

		const float x = (float)rand() / (float)RAND_MAX * 20.0f - 10.0f;
		const float y = (float)rand() / (float)RAND_MAX * 20.0f - 10.0f;
		const float z = (float)rand() / (float)RAND_MAX * 20.0f - 10.0f;
		Imath::V3f v(x,y,z);

		return anim::Transform(angle.toQuat(), v);
	}
}

BOOST_AUTO_TEST_CASE(transform_operations_random) {
	for(unsigned a=0;a<1000;++a) {
		anim::Transform t1 = randomTransform();
		anim::Transform t2 = randomTransform();

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				(t1 * t2).toMatrix44(),
				t1.toMatrix44() * t2.toMatrix44()
			),
			EPS
		);
	}

	for(unsigned a=0;a<1000;++a) {
		anim::Transform t1 = randomTransform();
		anim::Transform t2 = randomTransform();
		anim::Transform t3 = randomTransform();

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				(t1 * t2 * t3).toMatrix44(),
				t1.toMatrix44() * t2.toMatrix44() * t3.toMatrix44()
			),
			EPS
		);
	}
}

BOOST_AUTO_TEST_CASE(transform_operations_1) {
	///
	{
		Imath::Quatf q;
		q.setAxisAngle(Imath::V3f(0,0,1), 0);

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				q.toMatrix33(),
				Imath::M33f(1,0,0, 0,1,0, 0,0,1)
			),
			EPS
		);
	}


	for(unsigned a=0;a<100;++a) {
		float angle = 8.0f * M_PI * (float)rand() / (float)RAND_MAX;

		Imath::Quatf q;
		q.setAxisAngle(Imath::V3f(1,0,0), angle);

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				q.toMatrix33(),
				Imath::M33f(1,0,0, 0,cos(angle),sin(angle), 0,-sin(angle),cos(angle))
			),
			EPS
		);
	}

	for(unsigned a=0;a<100;++a) {
		float angle = 8.0f * M_PI * (float)rand() / (float)RAND_MAX;

		Imath::Quatf q;
		q.setAxisAngle(Imath::V3f(0,1,0), angle);

		BOOST_REQUIRE_SMALL(
			compareMatrices(
				q.toMatrix33(),
				Imath::M33f(cos(angle),0,-sin(angle), 0,1,0, sin(angle),0,cos(angle))
			),
			EPS
		);
	}
}
