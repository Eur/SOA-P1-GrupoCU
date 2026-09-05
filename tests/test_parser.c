#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "parser.h"
#include "double_linked_list.h"
#include "task.h"

/* ── Mini framework ─────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static int name(void)

#define ASSERT(cond, msg)                                      \
    do {                                                       \
        if (!(cond)) {                                         \
            fprintf(stderr, "  ASSERT FAILED: %s\n", (msg));  \
            return 1;                                          \
        }                                                      \
    } while (0)

#define RUN(name)                                              \
    do {                                                       \
        tests_run++;                                           \
        printf("[ RUN  ] %s\n", #name);                       \
        if (name() == 0) {                                     \
            printf("[ OK   ] %s\n", #name);                   \
            tests_passed++;                                    \
        } else {                                               \
            printf("[ FAIL ] %s\n", #name);                   \
            tests_failed++;                                    \
        }                                                      \
    } while (0)

/* ── Helper ─────────────────────────────────────────────────── */

static void write_csv(const char *path, const char *content)
{
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, "write_csv: could not create %s\n", path); return; }
    fputs(content, f);
    fclose(f);
}

/* ── Parser tests ───────────────────────────────────────────── */

TEST(test_entrada_valida_cinco_tareas)
{
    write_csv("/tmp/t_valid.csv",
        "task_id,tickets,work_units\n"
        "1,3,100\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_valid.csv", &head) == 0,
           "debe retornar 0 para entrada valida");

    int count = 0;
    for (struct node *c = head; c != NULL; c = c->next) count++;
    ASSERT(count == 5, "debe haber 5 nodos en la lista");

    ASSERT(head->id         == 1,   "primer nodo: id debe ser 1");
    ASSERT(head->tickets    == 3,   "primer nodo: tickets debe ser 3");
    ASSERT(head->work_units == 100, "primer nodo: work_units debe ser 100");

    dll_clean_list(&head);
    return 0;
}

TEST(test_orden_de_insercion_preservado)
{
    write_csv("/tmp/t_order.csv",
        "task_id,tickets,work_units\n"
        "10,1,10\n"
        "20,2,20\n"
        "30,3,30\n"
        "40,4,40\n"
        "50,5,50\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_order.csv", &head) == 0, "debe retornar 0");

    uint32_t expected[] = {10, 20, 30, 40, 50};
    int i = 0;
    for (struct node *c = head; c != NULL; c = c->next) {
        ASSERT(c->id == expected[i], "el orden debe coincidir con el CSV");
        i++;
    }

    dll_clean_list(&head);
    return 0;
}

TEST(test_menos_de_cinco_tareas)
{
    write_csv("/tmp/t_few.csv",
        "task_id,tickets,work_units\n"
        "1,3,100\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_few.csv", &head) == -1,
           "debe fallar con menos de 5 tareas");
    return 0;
}

TEST(test_mas_de_veinticinco_tareas)
{
    char csv[4096];
    int  off = snprintf(csv, sizeof(csv), "task_id,tickets,work_units\n");
    for (int i = 1; i <= 26; i++)
        off += snprintf(csv + off, sizeof(csv) - off, "%d,1,10\n", i);
    write_csv("/tmp/t_many.csv", csv);

    struct node *head;
    ASSERT(parser_load("/tmp/t_many.csv", &head) == -1,
           "debe fallar con mas de 25 tareas");
    return 0;
}

TEST(test_tickets_cero)
{
    write_csv("/tmp/t_tick0.csv",
        "task_id,tickets,work_units\n"
        "1,0,100\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_tick0.csv", &head) == -1,
           "debe fallar cuando tickets == 0");
    return 0;
}

TEST(test_tickets_negativos)
{
    write_csv("/tmp/t_tickneg.csv",
        "task_id,tickets,work_units\n"
        "1,-3,100\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_tickneg.csv", &head) == -1,
           "debe fallar cuando tickets es negativo");
    return 0;
}

TEST(test_work_units_cero)
{
    write_csv("/tmp/t_wu0.csv",
        "task_id,tickets,work_units\n"
        "1,3,0\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_wu0.csv", &head) == -1,
           "debe fallar cuando work_units == 0");
    return 0;
}

TEST(test_task_id_cero)
{
    write_csv("/tmp/t_id0.csv",
        "task_id,tickets,work_units\n"
        "0,3,100\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_id0.csv", &head) == -1,
           "debe fallar cuando task_id == 0");
    return 0;
}

TEST(test_id_duplicado)
{
    write_csv("/tmp/t_dup.csv",
        "task_id,tickets,work_units\n"
        "1,3,100\n"
        "1,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_dup.csv", &head) == -1,
           "debe fallar con task_id duplicado");
    return 0;
}

TEST(test_formato_invalido)
{
    write_csv("/tmp/t_fmt.csv",
        "task_id,tickets,work_units\n"
        "1,3\n"
        "2,5,200\n"
        "3,2,50\n"
        "4,4,80\n"
        "5,1,60\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_fmt.csv", &head) == -1,
           "debe fallar con formato invalido");
    return 0;
}

TEST(test_archivo_inexistente)
{
    struct node *head;
    ASSERT(parser_load("/tmp/no_existe_abc123.csv", &head) == -1,
           "debe fallar cuando el archivo no existe");
    return 0;
}

TEST(test_desbordamiento_total_tickets)
{
    write_csv("/tmp/t_overflow.csv",
        "task_id,tickets,work_units\n"
        "1,2147483648,10\n"
        "2,2147483648,10\n"
        "3,1,10\n"
        "4,1,10\n"
        "5,1,10\n");

    struct node *head;
    ASSERT(parser_load("/tmp/t_overflow.csv", &head) == -1,
           "debe fallar cuando la suma de tickets supera UINT32_MAX");
    return 0;
}

/* ── Task state machine tests ───────────────────────────────── */

TEST(test_task_create_estado_inicial)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    ASSERT(task->state           == TASK_READY, "estado inicial debe ser TASK_READY");
    ASSERT(task->dispatches      == 0,          "dispatches debe iniciar en 0");
    ASSERT(task->work_units_done == 0,          "work_units_done debe iniciar en 0");
    ASSERT(task->pi.j            == 0,          "pi.j debe iniciar en 0");
    task_destroy(task);
    return 0;
}

TEST(test_task_ready_to_running)
{
    task_t *task = task_create();
    ASSERT(task != NULL, "task_create debe retornar un puntero valido");
    ASSERT(task_transition_to_running(task) == true, "READY -> RUNNING debe ser exitoso");
    ASSERT(task->state      == TASK_RUNNING, "estado debe ser TASK_RUNNING");
    ASSERT(task->dispatches == 1,            "dispatches debe ser 1");
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
    ASSERT(task_transition_to_running(task)  == false, "FINISHED no puede volver a RUNNING");
    ASSERT(task_transition_to_ready(task)    == false, "FINISHED no puede volver a READY");
    ASSERT(task->state == TASK_FINISHED, "estado debe seguir siendo TASK_FINISHED");
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

    for (uint32_t i = 0; i < 1000000; i++){
        task_do_work_unit(t);
    }

    double error = t->pi.sum - 3.14159265358979323846;
    ASSERT(error < 1e-2 && error > -1e-2,
        "pi.sum debe converger a pi con tolerancia 1e-2");

    task_destroy(t);
    return 0;
}

/* ── Main ───────────────────────────────────────────────────── */

int main(void)
{
    printf("=== Parser — Pruebas Unitarias ===\n\n");

    RUN(test_entrada_valida_cinco_tareas);
    RUN(test_orden_de_insercion_preservado);
    RUN(test_menos_de_cinco_tareas);
    RUN(test_mas_de_veinticinco_tareas);
    RUN(test_tickets_cero);
    RUN(test_tickets_negativos);
    RUN(test_work_units_cero);
    RUN(test_task_id_cero);
    RUN(test_id_duplicado);
    RUN(test_formato_invalido);
    RUN(test_archivo_inexistente);
    RUN(test_desbordamiento_total_tickets);

    printf("\n=== Task — Pruebas Unitarias ===\n\n");

    RUN(test_task_create_estado_inicial);
    RUN(test_task_ready_to_running);
    RUN(test_task_running_to_ready);
    RUN(test_task_running_to_finished);
    RUN(test_task_finished_es_terminal);
    RUN(test_task_is_eligible);
    RUN(test_task_dispatches_acumulan);    
    
    printf("\n=== Pi — Pruebas Unitarias ===\n\n");

    RUN(test_pi_determinism);
    RUN(test_pi_convergence);

    printf("\n=== Resultado: %d/%d pruebas pasaron ===\n",
           tests_passed, tests_run);

    return (tests_failed > 0) ? 1 : 0;

}
