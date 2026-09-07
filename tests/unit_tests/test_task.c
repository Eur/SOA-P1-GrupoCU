// System imports
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Local project imports
#include "task.h"

// Unit test framework import
#include "unit_test_infra.h"

TEST(test_task_create_estado_inicial)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    ASSERT(task->state == TASK_READY, "estado inicial debe ser TASK_READY");
    ASSERT(task->work_units_done == 0, "work_units_done debe ser 0");
    ASSERT(task->dispatches == 0, "dispatches debe ser 0");
    ASSERT(task->pi.sum == 2.0, "pi.sum inicial debe ser 2.0");
    ASSERT(task->pi.term == 1.0, "pi.term inicial debe ser 1.0");
    ASSERT(task->pi.j == 0, "pi.j inicial debe ser 0");
    ASSERT(task->run_flag == false, "run_flag inicial debe ser false");
    ASSERT(task->done_flag == false, "done_flag inicial debe ser false");
    task_destroy(task);
    return 0;
}

TEST(test_task_ready_to_running)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    ASSERT(task_transition_to_running(task) == true, "READY -> RUNNING debe ser exitoso");
    ASSERT(task->state == TASK_RUNNING, "estado debe ser TASK_RUNNING");
    task_destroy(task);
    return 0;
}

TEST(test_task_running_to_ready)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    task_transition_to_running(task);
    ASSERT(task_transition_to_ready(task) == true, "RUNNING -> READY debe ser exitoso");
    ASSERT(task->state == TASK_READY, "estado debe ser TASK_READY");
    task_destroy(task);
    return 0;
}

TEST(test_task_running_to_finished)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    task_transition_to_running(task);
    ASSERT(task_transition_to_finished(task) == true, "RUNNING -> FINISHED debe ser exitoso");
    ASSERT(task->state == TASK_FINISHED, "estado debe ser TASK_FINISHED");
    task_destroy(task);
    return 0;
}

TEST(test_task_finished_es_terminal)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    task_transition_to_running(task);
    task_transition_to_finished(task);
    ASSERT(task_transition_to_running(task) == false, "FINISHED -> RUNNING debe fallar");
    ASSERT(task_transition_to_ready(task) == false, "FINISHED -> READY debe fallar");
    task_destroy(task);
    return 0;
}

TEST(test_task_is_eligible)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    ASSERT(task_is_eligible(task) == true, "tarea READY debe ser elegible");
    task_transition_to_running(task);
    ASSERT(task_is_eligible(task) == false, "tarea RUNNING no debe ser elegible");
    task_transition_to_finished(task);
    ASSERT(task_is_eligible(task) == false, "tarea FINISHED no debe ser elegible");
    task_destroy(task);
    return 0;
}

TEST(test_task_dispatches_acumulan)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    task_transition_to_running(task);
    task_transition_to_ready(task);
    task_transition_to_running(task);
    task_transition_to_ready(task);
    task_transition_to_running(task);
    ASSERT(task->dispatches == 3, "dispatches debe ser 3 tras tres sorteos ganados");
    task_destroy(task);
    return 0;
}

TEST(test_pi_determinism)
{
    task_t *a = task_create();
    task_t *b = task_create();
    ASSERT(a != NULL && b != NULL, "task_create debe retornar punteros validos");
    for (uint32_t i = 0; i < 1000; i++) {
        task_do_work_unit(a);
        task_do_work_unit(b);
    }
    ASSERT(a->pi.sum == b->pi.sum, "dos tareas con mismas iteraciones deben tener igual pi.sum");
    ASSERT(a->pi.j   == b->pi.j,   "dos tareas con mismas iteraciones deben tener igual pi.j");
    task_destroy(a);
    task_destroy(b);
    return 0;
}

TEST(test_pi_convergence)
{
    task_t *t = task_create();
    ASSERT(t != NULL, "task_create debe retornar un puntero valido");
    for (uint32_t i = 0; i < 1000000; i++)
        task_do_work_unit(t);
    double error = t->pi.sum - 3.14159265358979323846;
    ASSERT(error < 1e-2 && error > -1e-2,
           "pi.sum debe converger a pi con tolerancia 1e-2");
    task_destroy(t);
    return 0;
}

int main(void)
{
    printf("=== Task — Unit tests ===\n\n");

    RUN(test_task_create_estado_inicial);
    RUN(test_task_ready_to_running);
    RUN(test_task_running_to_ready);
    RUN(test_task_running_to_finished);
    RUN(test_task_finished_es_terminal);
    RUN(test_task_is_eligible);
    RUN(test_task_dispatches_acumulan);
    RUN(test_pi_determinism);
    RUN(test_pi_convergence);

    printf("\n=== Resultado: %d/%d pruebas pasaron ===\n",
           tests_passed, tests_run);

    return (tests_failed > 0) ? 1 : 0;
}