#pragma once
#include "glad.h"

void GLAPIENTRY glDebugOutput(
    GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

void enableReportGlErrors();

void createErrorFile();

void reportError(const char* message);
