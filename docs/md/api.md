
## Tests

* `CT_TEST(name) { ... }`

  Define a test named `name`.

  The code block that follows describes the body of the test. The body can
  contain any valid statements along with the assertions to be validated.

  `name` must be unique across all tests within a suite and a valid symbol
  (i.e., without surrounding quotes or any white space or special symbols not
  allowed in a variable name).

* `CT_TEST_WITH_FIXTURE(name, fixture_name) { ... }`

  Define a test named `name` that uses a fixture named `fixture_name`.

  See `CT_TEST` for details on `name` and the code block that must follow this
  macro.

  The fixture referenced by `fixture_name` must be defined, using `CT_FIXTURE`,
  prior to being referenced, otherwise a compiler error will be generated.

  Unlike tests defined by `CT_TEST`, a fixture will be initialized (set up)
  before the test is run and finalized (torn down) after the test has completed
  (whether successful or not). A pointer to the fixture can be accessed from the
  body of the test using the `fixture` variable.

* `CT_TEST_WITH_DATA(name, data_provider_name) { ... }`

  Define a test named `name` that is supplied data from the provider named
  `data_provider_name`.

  See `CT_TEST` for details on `name` and the code block that must follow this
  macro.

  The data provider referenced by `data_provider_name` must be defined, using
  `CT_DATA_PROVIDER`, prior to being referenced, otherwise a compiler error will
  be generated.

  Unlike a test defined by `CT_TEST` which only executes once, a test defined by
  `CT_TEST_WITH_DATA` will be executed once for each element supplied by the
  data provider. Each execution is contained in a single test case, with no data
  shared between cases. A pointer to the `struct` containing the data for a test
  case can be accessed from the body of the test using the `data` variable.

* `CT_TEST_WITH_FIXTURE_AND_DATA(name, fixture_name, data_provider_name) { ...  }`

  Define a test named `name` that uses a fixture named `fixture_name` and is
  supplied data from the provider named `data_provider_name`.

  See `CT_TEST` for details on `name` and the code block that must follow this
  macro.

  See `CT_TEST_WITH_FIXTURE` and `CT_TEST_WITH_DATA` for details on how the data
  provider and fixture behave. The fixture and data provider are independent
  from one another. This means that each test case will be supplied it's own
  fixture, with no data shared between test cases.

## Data Providers

* `CT_DATA_TYPE(name) { ... };`

  Define a `struct`-like type for the data provider named `name` for providing
  data to tests.

  The `struct` declaration that follows (enclosed with braces) describes the
  data to be provided. Just like a `struct` definition, the closing brace must
  be followed by a semicolon (`;`).

  See `CT_DATA_PROVIDER` for details on `name`.

* `CT_DATA(name) { ... };`

  Define the data to be supplied by the data provider named `name`.

  The expression that follows must conform to the syntax for the initialization
  of an array of `struct`s. Each element of the array will be of the type
  defined by the `CT_DATA_TYPE` associated with the same provider (i.e., with
  the same name).

  See `CT_DATA_PROVIDER` for details on `name`.

* `CT_DATA_PROVIDER(name, fmt, ...);`

   Define a data provider named `name`.

   `name` must be unique across all data providers within a suite and a valid
   symbol (i.e., without surrounding quotes or any white space or special
   symbols not allowed in a variable name).

   `fmt, ...` is a `printf`-style formatting string as associated parameters
   that can create a human-readable string from an element supplied by the data
   provider. A pointer to the element can be referenced by the formatting
   parameters using the `data` variable. The generated string is used to augment
   the test case names of tests using this provider.

   The type of data this provider provides is defined by the data provider
   type of the same name (i.e., the `CT_DATA_TYPE` statement using the same name
   as this provider).

   The data this provider provides is defined by the data provider data of the
   same name (.e., the `CT_DATA` statement using the same name as this
   provider).

## Fixtures

* `CT_FIXTURE_TYPE(name) { ... };`

  Define a `struct`-like type for the fixture named `name`.

  The `struct` declaration that follows (enclosed with braces) describes the
  state of the fixture. Just like a `struct` definition, the closing brace must
  be followed by a semicolon (`;`).

  See `CT_FIXTURE` for details on `name`.

* `CT_FIXTURE_SETUP(name) { ... }`

  Define the set up routine for the fixture named `name`.

  The code-block that follows describes the initialization (set up) to be
  performed on the fixture. The body can contain any valid statements and can
  refer to the fixture being initialized using the `fixture` variable.

  Any assertion (e.g., `CT_ASSERT`, `CT_FAIL`, etc.) can be used in the set up
  routine to abort the test (with a failure). Failure during fixture setup is
  treated differently by ctest.

  See `CT_FIXTURE` for details on `name`.

* `CT_FIXTURE_TEARDOWN(name) { ... }`

  Define the tear down routine for the fixture named `name`.

  The code-block that follows describes the finalization (tear down) to be
  performed on the fixture. The body can contain any valid statements and can
  refer to the fixture being finalized using the `fixture` variable.

  Any assertion (e.g., `CT_ASSERT`, `CT_FAIL`, etc.) can be used in the tear
  down routine to cause the test to fail.

  See `CT_FIXTURE` for details on `name`.

* `CT_FIXTURE(name);`

  Defines a fixture named `name`.

   `name` must be unique across all fixtures within a suite and a valid symbol
   (i.e., without surrounding quotes or any white space or special symbols not
   allowed in a variable name).

  The type of this fixture is described by the fixture type definition with the
  same name (i.e., the `CT_FIXTURE_TYPE` statement using the same name as this
  fixture) with set up and tear down routines also with the same name (i.e., the
  `CT_FIXTURE_SETUP` and `CT_FIXTURE_TEARDOWN` statements using the same name as
  this fixture). All three of these definitions are required.

## Assertions

All assertion macros can optionally be supplied `printf`-style format string
(and associated parameters) describing additional information about the failure
(e.g., providing additional context).

Several of the assertion macros compare the actual value generated to an
expected value. These macros can be grouped together based on the type of
values they compare. Within each group, the last two letters of the assertion
macro describe the type of comparison:

| Suffix | Fail unless ...                                                  |
|:-------|:-----------------------------------------------------------------|
| `EQ`   | the actual value is equal to the expected value.                 |
| `NE`   | the actual value is not equal to the expected value.             |
| `LT`   | the actual value is less than the expected value.                |
| `LE`   | the actual value is less than or equal to the expected value.    |
| `GT`   | the actual value is greater than the expected value.             |
| `GE`   | the actual value is greater than or equal to the expected value. |

* `CT_FAIL([fmt, ...])`

  Unconditionally fail a test.

* `CT_ASSERT(expr[, fmt, ...])`

  Fail a test if `expr` evaluates to `false`.

  If not specified, a generic error will be generated.

* `CT_ASSERT_UINT_EQ(act, exp[, fmt, ...])`

  `CT_ASSERT_UINT_NE(act, exp[, fmt, ...])`

  `CT_ASSERT_UINT_LT(act, exp[, fmt, ...])`

  `CT_ASSERT_UINT_LE(act, exp[, fmt, ...])`

  `CT_ASSERT_UINT_GT(act, exp[, fmt, ...])`

  `CT_ASSERT_UINT_GE(act, exp[, fmt, ...])`

  Fail a test if the actual (`act`) integer value does not compare correctly to
  the expected (`exp`) integer value.

  Both `act` and `exp` are treated as unsigned.

* `CT_ASSERT_INT_EQ(act, exp[, fmt, ...])`

  `CT_ASSERT_INT_NE(act, exp[, fmt, ...])`

  `CT_ASSERT_INT_LT(act, exp[, fmt, ...])`

  `CT_ASSERT_INT_LE(act, exp[, fmt, ...])`

  `CT_ASSERT_INT_GT(act, exp[, fmt, ...])`

  `CT_ASSERT_INT_GE(act, exp[, fmt, ...])`

  Fail a test if the actual (`act`) integer value does not compare correctly to
  the expected (`exp`) integer value.

  Both `act` and `exp` are treated as signed.

* `CT_ASSERT_STR_EQ(act, exp[, fmt, ...])`

  `CT_ASSERT_STR_NE(act, exp[, fmt, ...])`

  `CT_ASSERT_STR_LT(act, exp[, fmt, ...])`

  `CT_ASSERT_STR_LE(act, exp[, fmt, ...])`

  `CT_ASSERT_STR_GT(act, exp[, fmt, ...])`

  `CT_ASSERT_STR_GE(act, exp[, fmt, ...])`

  Fail a test if the actual (`act`) string value does not compare correctly to
  the expected (`exp`) string value.

  Comparisons are case sensitive and inequalities are compared are in
  lexicographical order.

* `CT_ASSERT_ISTR_EQ(act, exp[, fmt, ...])`

  `CT_ASSERT_ISTR_NE(act, exp[, fmt, ...])`

  `CT_ASSERT_ISTR_LT(act, exp[, fmt, ...])`

  `CT_ASSERT_ISTR_LE(act, exp[, fmt, ...])`

  `CT_ASSERT_ISTR_GT(act, exp[, fmt, ...])`

  `CT_ASSERT_ISTR_GE(act, exp[, fmt, ...])`

  Fail a test if the actual (`act`) string value does not compare correctly to
  the expected (`exp`) string value.

  Comparisons are case insensitive and inequalities are compared are in
  lexicographical order.

* `CT_ASSERT_BOOL_EQ(act, exp[, fmt, ...])

  `CT_ASSERT_BOOL_NE(act, exp[, fmt, ...])

  Fail a test if the actual (`act`) boolean value does not compare correctly to
  the expected (`exp`) boolean value.

  Both `act` and `exp` are treated as boolean values (i.e., a zero is only equal
  to zero and a non-zero is equal to any non-zero).

* `CT_ASSERT_TRUE(act[, fmt, ...])`

  Fail a test if `act` is not equivalent to `true`.

  Any non-zero/non-`NULL` value is considered `true`.

  This is effectively equivalent to `CT_ASSERT(act)` and
  `CT_ASSERT_BOOL_EQ(act,true)`

* `CT_ASSERT_FALSE(act[, fmt, ...])`

  Fail a test if `act` is not equivalent to `false`.

  Any zero/`NULL` value is considered `false`.

  This is effectively equivalent to `CT_ASSERT(!act)` and
  `CT_ASSERT_BOOL_EQ(act,false)`
