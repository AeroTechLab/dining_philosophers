//
// Created by grilo on 03/05/2020.
//

#ifndef COLORS_H
#define COLORS_H

#define _RED     "\x1b[31m"
#define _GREEN   "\x1b[32m"
#define _YELLOW  "\x1b[33m"
#define _BLUE    "\x1b[34m"
#define _MAGENTA "\x1b[35m"
#define _CYAN    "\x1b[36m"
#define _RESET   "\x1b[0m"

#define RED(string) _RED string _RESET
#define GREEN(string) _GREEN string _RESET
#define YELLOW(string) _YELLOW string _RESET
#define BLUE(string) _BLUE string _RESET
#define MAGENTA(string) _MAGENTA string _RESET
#define CYAN(string) _CYAN string _RESET

#endif //COLORS_H
