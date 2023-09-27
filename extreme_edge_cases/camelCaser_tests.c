/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **))
{
    char **outputs = NULL;

    //*************************************************TESTCASE1************************************************************
    outputs = (*camelCaser)("TheHeisenbug.");
    if (!(strcmp("theheisenbug", outputs[0]) == 0 && outputs[1] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE2************************************************************
    outputs = (*camelCaser)("The Heisenbug incredible creature. Facenovel servers. Code smell INCREDIBLE use of air freshener.");
    if (!(strcmp("theHeisenbugIncredibleCreature", outputs[0]) == 0 && strcmp("facenovelServers", outputs[1]) == 0 && strcmp("codeSmellIncredibleUseOfAirFreshener", outputs[2]) == 0 && outputs[3] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE3************************************************************
    outputs = (*camelCaser)("T    his i s AteSt,      odwnj");
    if (!(strcmp("tHisISAtest", outputs[0]) == 0 && outputs[1] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE4************************************************************
    outputs = (*camelCaser)("Hello\x1fWorld.");
    if (!(strcmp(outputs[0], "hello\x1fworld") == 0 && outputs[1] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE5************************************************************
    outputs = (*camelCaser)(NULL);
    if (!(outputs == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE6************************************************************
    outputs = (*camelCaser)("");
    if (outputs[0] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE7************************************************************
    outputs = (*camelCaser)("!   ?!o!");
    if (!(strcmp(outputs[0], "") == 0 && strcmp(outputs[1], "") == 0 && strcmp(outputs[2], "") == 0 && strcmp(outputs[3], "o") == 0 && outputs[4] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE8***********************************************************
    outputs = (*camelCaser)("\\");
    if (!(strcmp(outputs[0], "") == 0 && outputs[1] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE9***********************************************************
    outputs = (*camelCaser)("\v 341    \f  cs. Is \t   SO \n INTERESTING! \r");
    if (!(strcmp(outputs[0],"341Cs") == 0 && strcmp(outputs[1],"isSoInteresting") == 0 && outputs[2] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE10**********************************************************
    outputs = (*camelCaser)("empty");
    if (outputs[0] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE11**********************************************************
    outputs = (*camelCaser)("!#$%&()'*+,-./:;?@[]^_`{|}~punct");
    size_t i;
    for (i = 0; i < 27; i++) {
        if (strcmp(outputs[i],"") != 0) {
            destroy(outputs);
            return 0;
        }
    }
    if (!(outputs[27] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    //*************************************************TESTCASE12**********************************************************
    outputs = (*camelCaser)("(·•᷄ࡇ•᷅ )");
    if (!(strcmp(outputs[0], "") == 0 && strcmp(outputs[1], "·•᷄ࡇ•᷅") == 0) && (outputs[2] == NULL)) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    return 1;
}