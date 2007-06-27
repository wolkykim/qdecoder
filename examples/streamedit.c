/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

#include "qDecoder.h"

#define SOURCE		"streamedit.html.in"

int main(void) {
  Q_ENTRY *args;
  char *name, *hobby;

  qContentType("text/html");

  name = qValueDefault("Not Found", "name");
  hobby = qValueDefault("Not Found", "hobby");

  args = NULL;
  args = qSedArgAdd(args, "${NAME}", name);
  args = qSedArgAdd(args, "${HOBBY}", hobby);
  if(qSedFile(args, SOURCE, stdout) == 0) qError("File(%s) not found.", SOURCE);
  qSedArgFree(args);

  return 0;
}

