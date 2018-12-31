#include <boost/test/unit_test.hpp>

#include <actions/undo_stack.h>

namespace std {
	std::ostream& operator << (std::ostream& out, const std::vector<unsigned>& vals) {
		return out;
	}
}

BOOST_AUTO_TEST_CASE(simple_undo_redo) {
	std::vector<unsigned> values;

	possumwood::UndoStack stack;

	{
		possumwood::UndoStack::Action a;
		a.addCommand(
			"Pushing back 10",
			[&values]() {
				values.push_back(10);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 10u);
				values.pop_back();
			}
		);
		stack.execute(a);
	}

	BOOST_CHECK_EQUAL(values, decltype(values){10});

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, decltype(values){});

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, decltype(values){});

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, decltype(values){10});

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, decltype(values){10});

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, decltype(values){});

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, decltype(values){10});
}

BOOST_AUTO_TEST_CASE(multi_command_undo_redo) {
	std::vector<unsigned> values;

	possumwood::UndoStack stack;

	{
		possumwood::UndoStack::Action a;
		a.addCommand(
			"Pushing back 10",
			[&values]() {
				values.push_back(10);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 10u);
				values.pop_back();
			}
		);

		a.addCommand(
			"Pushing back 10",
			[&values]() {
				values.push_back(20);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 20u);
				values.pop_back();
			}
		);

		stack.execute(a);
	}

	const std::vector<unsigned> expected_result{10, 20};

	BOOST_CHECK_EQUAL(values, expected_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, decltype(values){});

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, decltype(values){});

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, expected_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, expected_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, decltype(values){});

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, expected_result);
}

BOOST_AUTO_TEST_CASE(multi_action_undo_redo) {
	std::vector<unsigned> values;

	possumwood::UndoStack stack;

	{
		possumwood::UndoStack::Action a;
		a.addCommand(
			"Pushing back 10",
			[&values]() {
				values.push_back(10);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 10u);
				values.pop_back();
			}
		);

		a.addCommand(
			"Pushing back 20",
			[&values]() {
				values.push_back(20);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 20u);
				values.pop_back();
			}
		);

		stack.execute(a);
	}

	{
		possumwood::UndoStack::Action a;
		a.addCommand(
			"Pushing back 30",
			[&values]() {
				values.push_back(30);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 30u);
				values.pop_back();
			}
		);

		a.addCommand(
			"Pushing back 40",
			[&values]() {
				values.push_back(40);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 40u);
				values.pop_back();
			}
		);

		stack.execute(a);
	}
	const std::vector<unsigned> full_result{10, 20, 30, 40};
	const std::vector<unsigned> half_result{10, 20};
	const std::vector<unsigned> empty_result{};

	BOOST_CHECK_EQUAL(values, full_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, full_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, full_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, full_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, full_result);
}

BOOST_AUTO_TEST_CASE(exception_handling) {
	std::vector<unsigned> values;

	// 30 will never be inserted - need only 10 and 20
	const std::vector<unsigned> half_result{10, 20};
	const std::vector<unsigned> empty_result{};

	possumwood::UndoStack stack;

	BOOST_CHECK_EQUAL(values, empty_result);

	{

		possumwood::UndoStack::Action a;
		a.addCommand(
			"Pushing back 10",
			[&values]() {
				values.push_back(10);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 10u);
				values.pop_back();
			}
		);

		a.addCommand(
			"Pushing back 20",
			[&values]() {
				values.push_back(20);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 20u);
				values.pop_back();
			}
		);

		BOOST_REQUIRE_NO_THROW(stack.execute(a));
	}

	BOOST_CHECK_EQUAL(values, half_result);

	{
		possumwood::UndoStack::Action a;
		a.addCommand(
			"Pushing back 30",
			[&values]() {
				values.push_back(30);

				const std::vector<unsigned> tmp{10, 20, 30};
				BOOST_CHECK_EQUAL(values, tmp);
			},
			[&values]() {
				BOOST_CHECK_EQUAL(values.back(), 30u);
				values.pop_back();

				const std::vector<unsigned> tmp{10, 20};
				BOOST_CHECK_EQUAL(values, tmp);
			}
		);

		a.addCommand(
			"Throwing",
			[]() {
				throw "stuff";
			},
			[]() {
			}
		);

		// executing this command throws, and UNROLLS push_back of 30
		BOOST_REQUIRE_THROW(stack.execute(a), const char*);
		BOOST_REQUIRE_THROW(stack.execute(a), const char*);
	}

	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.undo());
	BOOST_CHECK_EQUAL(values, empty_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, half_result);

	BOOST_REQUIRE_NO_THROW(stack.redo());
	BOOST_CHECK_EQUAL(values, half_result);
}
