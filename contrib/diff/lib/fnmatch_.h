/* Copyright (C) 1991, 1992, 1993, 1996, 1997, 1998, 1999, 2001, 2002, 2003
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef	_FNMATCH_H
# define _FNMATCH_H	1

# ifdef	__cplusplus
extern "C" {
# endif

/* We #undef these before defining them because some losing systems
   (HP-UX A.08.07 for example) define these in <unistd.h>.  */
# undef	FNM_PATHNAME
# undef	FNM_NOESCAPE
# undef	FNM_PERIOD

/* Bits set in the FLAGS argument to `fnmatch'.  */
# define FNM_PATHNAME	(1 << 0) /* No wildcard can ever match `/'.  */
# define FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars.  */
# define FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly.  */

# if !defined _POSIX_C_SOURCE || _POSIX_C_SOURCE < 2 || defined _GNU_SOURCE
#  define FNM_FILE_NAME	 FNM_PATHNAME	/* Preferred GNU name.  */
#  define FNM_LEADING_DIR (1 << 3)	/* Ignore `/...' after a match.  */
#  define FNM_CASEFOLD	 (1 << 4)	/* Compare without regard to case.  */
#  define FNM_EXTMATCH	 (1 << 5)	/* Use ksh-like extended matching. */
# endif

/* Value returned by `fnmatch' if STRING does not match PATTERN.  */
# define FNM_NOMATCH	1

/* This value is returned if the implementation does not support
   `fnmatch'.  Since this is not the case here it will never be
   returned but the conformance test suites still require the symbol
   to be defined.  */
# ifdef _XOPEN_SOURCE
#  define FNM_NOSYS	(-1)
# endif

/* Match NAME against the filename pattern PATTERN,
   returning zero if it matches, FNM_NOMATCH if not.  */
extern int fnmatch (const char *__pattern, const char *__name,
		    int __flags);

# ifdef	__cplusplus
}
# endif

#endif /* fnmatch.h */
