#ifndef CTEST__EXEC__EXEC_HOOKS_H__INCLUDED__
#define CTEST__EXEC__EXEC_HOOKS_H__INCLUDED__

#include <ctest/_annotations.h>
#include <ctest/exec/result.h>
#include <ctest/exec/stage.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Execution hooks for receiving feedback during the execution of an test case.
 *
 * When test cases are executed, they need a method for asynchronously reporting
 * state (such as an assertion failure). These hooks allow different runners
 * (or whatever is executing a test case) to have its own implementation for
 * dealing with events.
 *
 * Note that there are is no explicit succeeded event; success is implicit
 * when the function for executing a test case returns.
 */
typedef struct ctest_exec_hooks ctest_exec_hooks_t;
typedef const struct ctest_exec_hooks_ops ctest_exec_hooks_ops_t;
struct ctest_exec_hooks_ops {
	void (*on_stage_change)(ctest_exec_hooks_t *, ctest_stage_t);

	CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
	void (*on_skip)(ctest_exec_hooks_t *, ctest_failure_t *);

	CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
	void (*on_failure)(ctest_exec_hooks_t *, ctest_failure_t *);
};
struct ctest_exec_hooks {
	ctest_exec_hooks_ops_t *ops;
};

/**
 * Report a transition into a new stage of execution.
 *
 * Execution of a test progresses (monotonically increasingly) through several
 * stages. This handler is for reporting these transitions.
 *
 * @param hooks The hooks to handle the skipped test case.
 * @param stage The stage of execution that has begun.
 */
static inline void ctest_exec_hooks_on_stage_change(ctest_exec_hooks_t *hooks, ctest_stage_t stage)
{
	(*hooks->ops->on_stage_change)(hooks, stage);
}

/**
 * Report the associated test case as being skipped.
 *
 * A skipped test is one that should be considered as not executed, so it
 * neither passed nor failed. For example, if a test requires the presence of
 * some file that does not exist, the test can be skipped rather than failed.
 * The reporter can then choose how to handle these warnings.
 *
 * This function does not return.
 *
 * @param hooks   The hooks to handle the skipped test case.
 * @param failure The "failure" that triggered the skipped test. Ownership of
 *                this failure is transferred to the callee.
 */
CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
static inline void ctest_exec_hooks_on_skip(ctest_exec_hooks_t *hooks, ctest_failure_t *failure)
{
	(*hooks->ops->on_skip)(hooks, failure);
}

/**
 * Report the associated test case as having failed.
 *
 * This function does not return.
 *
 * @param hooks   The hooks to handle the failed test case.
 * @param failure The failure that occurred while the test was being set up.
 *                Ownership of this failure is transferred to the callee.
 */
CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
static inline void ctest_exec_hooks_on_failure(ctest_exec_hooks_t *hooks, ctest_failure_t *failure)
{
	(*hooks->ops->on_failure)(hooks, failure);
}

#ifdef __cplusplus
}
#endif

#endif /* CTEST__EXEC__EXEC_HOOKS_H__INCLUDED__ */
