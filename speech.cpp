/***************************************************************************
                          speech.cpp  -  description
                             -------------------
    begin                : Son Sep 8 2002
    copyright            : (C) 2002 by Gunnar Schmi Dt
    email                : kmouth@schmi-dt.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// $Id$

#include "speech.h"
#include <qstring.h>
#include <qvaluelist.h>
#include <qvaluestack.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <kdebug.h>

Speech::Speech() {
}

Speech::~Speech() {
}

QString Speech::prepareCommand (QString command, QString text) {
   QValueStack<bool> stack;  // saved isdoublequote values during parsing of braces
   bool issinglequote=false; // inside '...' ?
   bool isdoublequote=false; // inside "..." ?
   int noreplace=0; // nested braces when within ${...}

   // character sequences that change the state or need to be otherwise processed
   QRegExp re_singlequote("('|%%|%t)");
   QRegExp re_doublequote("(\"|\\\\|`|\\$\\(|\\$\\{|%%|%t)");
   QRegExp re_noquote  ("('|\"|\\\\|`|\\$\\(|\\$\\{|\\(|\\{|\\)|\\}|%%|%t)");

   // parse the command:
   for (int i = re_noquote.search(command);
        i != -1;
        i = (issinglequote?re_singlequote.search(command,i)
            :isdoublequote?re_doublequote.search(command,i)
            :re_noquote.search(command,i))
       )
      // while there are character sequences that need to be processed
   {
      if ((command[i]=='(') || (command[i]=='{')) { // (...) or {...}
         // assert(isdoublequote == false)
         stack.push(isdoublequote);
         if (noreplace > 0)
            // count nested braces when within ${...}
            noreplace++;
         i++;
      }
      else if (command[i]=='$') { // $(...) or ${...}
         stack.push(isdoublequote);
         isdoublequote = false;
         if ((noreplace > 0) || (command[i+1]=='{'))
            // count nested braces when within ${...}
            noreplace++;
         i+=2;
      }
      else if ((command[i]==')') || (command[i]=='}')) {
         // $(...) or (...) or ${...} or {...}
         if (!stack.isEmpty())
            isdoublequote = stack.pop();
         else
            qWarning("Parse error.");
         if (noreplace > 0)
            // count nested braces when within ${...}
            noreplace--;
         i++;
      }
      else if (command[i]=='\'') {
         issinglequote=!issinglequote;
         i++;
      }
      else if (command[i]=='"') {
         isdoublequote=!isdoublequote;
         i++;
      }
      else if (command[i]=='\\')
         i+=2;
      else if (command[i]=='`') {
         // Replace all `...` with safer $(...)
         command.replace (i, 1, "$(");
         QRegExp re_backticks("(`|\\\\`|\\\\\\\\|\\\\\\$)");
         for (int i2=re_backticks.search(command,i+2);
              i2!=-1;
              i2=re_backticks.search(command,i2)
             )
         {
            if (command[i2] == '`') {
               command.replace (i2, 1, ")");
               i2=command.length(); // leave loop
            }
            else {
               // remove backslash and ignore following character
               command.remove (i2, 1);
               i2++;
            }
         }
         // Leave i unchanged! We need to process "$("
      }
      else if (noreplace > 0) { // do not replace macros within ${...}
         if (issinglequote)
            i+=re_singlequote.matchedLength();
         else if (isdoublequote)
            i+=re_doublequote.matchedLength();
         else
            i+=re_noquote.matchedLength();
      }
      else { // replace macro
         QString match, v;

         // get match
         if (issinglequote)
            match=re_singlequote.cap();
         else if (isdoublequote)
            match=re_doublequote.cap();
         else
            match=re_noquote.cap();

         // substitute %variables
         if (match=="%t")
            v = text;
         else if (match=="%%")
            v = "%";

         // %variable inside of a quote?
         if (isdoublequote)
            v='"'+v+'"';
         else if (issinglequote)
            v="'"+v+"'";

         command.replace (i, match.length(), v);
         i+=v.length();
      }
   }
   return command;
}

void Speech::speak(QString command, bool stdIn, QString text, int encoding, QTextCodec *codec) {
   if (text.length () > 0) {
      // 1. prepare the text:
      // 1.a) encode the text
      QTextStream ts (encText, IO_WriteOnly);
      if (encoding == Local)
         ts.setEncoding (QTextStream::Locale);
      else if (encoding == Latin1)
         ts.setEncoding (QTextStream::Latin1);
      else if (encoding == Unicode)
         ts.setEncoding (QTextStream::Unicode);
      else
         ts.setCodec (codec);
      ts << text;

      // 1.b) quote the text as one parameter
      QString escText = KShellProcess::quote(encText);

      // 2. prepare the command:
      command = prepareCommand (command, escText);


      // 3. create a new process
      process << command;
      connect(&process, SIGNAL(processExited(KProcess *)), this, SLOT(processExited(KProcess *)));
      connect(&process, SIGNAL(wroteStdin(KProcess *)), this, SLOT(wroteStdin(KProcess *)));
      connect(&process, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(receivedStdout(KProcess *, char *, int)));
      connect(&process, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(receivedStderr(KProcess *, char *, int)));

      // 4. start the process
      if (stdIn) {
         process.start(KProcess::NotifyOnExit, KProcess::All);
         if (encText.size() > 0)
            process.writeStdin(encText, encText.size());
         else
            process.closeStdin();
      }
      else
         process.start(KProcess::NotifyOnExit, KProcess::AllOutput);
   }
}

void Speech::receivedStdout (KProcess *, char *buffer, int buflen) {
   kdDebug() << QString::fromLatin1(buffer, buflen) + "\n";
}
void Speech::receivedStderr (KProcess *, char *buffer, int buflen) {
   kdDebug() << QString::fromLatin1(buffer, buflen) + "\n";
}

void Speech::wroteStdin(KProcess *) {
   process.closeStdin();
}

void Speech::processExited(KProcess *) {
   delete this;
}

#include "speech.moc"

/*
 * $Log$
 * Revision 1.9  2002/12/16 10:15:47  gunnar
 * Secutity fix
 *
 * Revision 1.8  2002/12/16 10:04:12  gunnar
 * Secutity fix
 *
 * Revision 1.7  2002/12/11 18:57:31  gunnar
 * Incorporated changes for version 0.5.2 into head branch
 *
 * Revision 1.3.2.2  2002/12/11 18:31:23  gunnar
 * Security fix
 *
 * Revision 1.6  2002/12/04 16:22:02  gunnar
 * Include *.moc files
 *
 * Revision 1.5  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.4  2002/11/04 16:38:42  gunnar
 * Incorporated changes for version 0.5.1 into head branch
 *
 * Revision 1.3.2.1  2002/11/04 15:36:37  gunnar
 * combo box for character encoding added
 *
 * Revision 1.3  2002/10/02 16:55:17  gunnar
 * Removed some compiler warnings and implementing the feature of opening a history at the start of KMouth
 *
 * Revision 1.2  2002/10/02 14:55:33  gunnar
 * Fixed Speak-empty-phrase-crash bug and added some i18n() encodings
 *
 * Revision 1.1  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 */
