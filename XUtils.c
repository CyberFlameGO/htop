/*
htop - StringUtils.c
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "config.h" // IWYU pragma: keep

#include "XUtils.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "CRT.h"


void fail() {
   CRT_done();
   abort();

   _exit(1); // Should never reach here
}

void* xMalloc(size_t size) {
   assert(size > 0);
   void* data = malloc(size);
   if (!data) {
      fail();
   }
   return data;
}

void* xCalloc(size_t nmemb, size_t size) {
   assert(nmemb > 0);
   assert(size > 0);
   void* data = calloc(nmemb, size);
   if (!data) {
      fail();
   }
   return data;
}

void* xRealloc(void* ptr, size_t size) {
   assert(size > 0);
   void* data = realloc(ptr, size); // deepcode ignore MemoryLeakOnRealloc: this goes to fail()
   if (!data) {
      free(ptr);
      fail();
   }
   return data;
}

char* String_cat(const char* s1, const char* s2) {
   const size_t l1 = strlen(s1);
   const size_t l2 = strlen(s2);
   char* out = xMalloc(l1 + l2 + 1);
   memcpy(out, s1, l1);
   memcpy(out + l1, s2, l2);
   out[l1 + l2] = '\0';
   return out;
}

char* String_trim(const char* in) {
   while (in[0] == ' ' || in[0] == '\t' || in[0] == '\n') {
      in++;
   }

   size_t len = strlen(in);
   while (len > 0 && (in[len - 1] == ' ' || in[len - 1] == '\t' || in[len - 1] == '\n')) {
      len--;
   }

   return xStrndup(in, len);
}

char** String_split(const char* s, char sep, size_t* n) {
   const unsigned int rate = 10;
   char** out = xCalloc(rate, sizeof(char*));
   size_t ctr = 0;
   unsigned int blocks = rate;
   const char* where;
   while ((where = strchr(s, sep)) != NULL) {
      size_t size = (size_t)(where - s);
      out[ctr] = xStrndup(s, size);
      ctr++;
      if (ctr == blocks) {
         blocks += rate;
         out = (char**) xRealloc(out, sizeof(char*) * blocks);
      }
      s += size + 1;
   }
   if (s[0] != '\0') {
      out[ctr] = xStrdup(s);
      ctr++;
   }
   out = xRealloc(out, sizeof(char*) * (ctr + 1));
   out[ctr] = NULL;

   if (n)
      *n = ctr;

   return out;
}

void String_freeArray(char** s) {
   if (!s) {
      return;
   }
   for (size_t i = 0; s[i] != NULL; i++) {
      free(s[i]);
   }
   free(s);
}

char* String_getToken(const char* line, const unsigned short int numMatch) {
   const size_t len = strlen(line);
   char inWord = 0;
   unsigned short int count = 0;
   char match[50];

   size_t foundCount = 0;

   for (size_t i = 0; i < len; i++) {
      char lastState = inWord;
      inWord = line[i] == ' ' ? 0 : 1;

      if (lastState == 0 && inWord == 1)
         count++;

      if (inWord == 1) {
         if (count == numMatch && line[i] != ' ' && line[i] != '\0' && line[i] != '\n' && line[i] != (char)EOF) {
            match[foundCount] = line[i];
            foundCount++;
         }
      }
   }

   match[foundCount] = '\0';
   return xStrdup(match);
}

char* String_readLine(FILE* fd) {
   const unsigned int step = 1024;
   unsigned int bufSize = step;
   char* buffer = xMalloc(step + 1);
   char* at = buffer;
   for (;;) {
      char* ok = fgets(at, step + 1, fd);
      if (!ok) {
         free(buffer);
         return NULL;
      }
      char* newLine = strrchr(at, '\n');
      if (newLine) {
         *newLine = '\0';
         return buffer;
      } else {
         if (feof(fd)) {
            return buffer;
         }
      }
      bufSize += step;
      buffer = xRealloc(buffer, bufSize + 1);
      at = buffer + bufSize - step;
   }
}

int xAsprintf(char** strp, const char* fmt, ...) {
   va_list vl;
   va_start(vl, fmt);
   int r = vasprintf(strp, fmt, vl);
   va_end(vl);

   if (r < 0 || !*strp) {
      fail();
   }

   return r;
}

int xSnprintf(char* buf, int len, const char* fmt, ...) {
   va_list vl;
   va_start(vl, fmt);
   int n = vsnprintf(buf, len, fmt, vl);
   va_end(vl);

   if (n < 0 || n >= len) {
      fail();
   }

   return n;
}

char* xStrdup(const char* str) {
   char* data = strdup(str);
   if (!data) {
      fail();
   }
   return data;
}

char* xStrndup(const char* str, size_t len) {
   char* data = strndup(str, len);
   if (!data) {
      fail();
   }
   return data;
}
