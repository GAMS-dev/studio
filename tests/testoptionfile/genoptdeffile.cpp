/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* This code is to generate an option definition file 'optdummy.def'
 * used for GAMS Studio unit test.
 * To compile the code, download the include files from 'coin-or/GAMSLinks'
 * project at https://github.com/coin-or/GAMSlinks/tree/master/src/utils
*/
#include "GamsLinksConfig.h"
#include "GamsOptionsSpecWriter.hpp"

int main(int argc, char** argv)
{
   GamsOptions gmsopt("Dummy");
   gmsopt.setSeparator("=");
   gmsopt.setStringQuote("\"");
   gmsopt.setGroup("FirstGroup");
   gmsopt.setEolChars("#");

   for( int i = 0; i < 4; ++i )
   {
      gmsopt.collect(
         std::string("bool_") + std::to_string(i),
         std::string("description for option bool_")+std::to_string( i ), std::string(),
         (bool)(i%2==0? false :true) );
   }

   for( int i = 0; i < 5; ++i )
   {
      int defaultval = i;
      int minval = 0-i;
      int maxval = 10 + i;
      std::string defaultdescr("default description");
      GamsOption& opt( gmsopt.collect(
            std::string("int_")+std::to_string( i ),
            std::string("description for option int_")+std::to_string( i ), std::string(),
            defaultval, minval, maxval, defaultdescr) );
      if (i==1)
         opt.hidden = true;
   }

   for( int i = 1; i < 6; ++i )
   {
      GamsOption::EnumVals enumint;
      for( int j = 0; j < i*2; ++j)
          enumint.append( j+1, std::string("enumint_")+std::to_string(i)+std::string("_")+std::to_string(j) );
      std::string defaultdescr("default description");
      GamsOption& opt( gmsopt.collect(
            std::string("EnumInt_")+std::to_string( i ),
            std::string("description for option EnumInt_")+std::to_string( i ), std::string(),
            i,  enumint, defaultdescr) );
      if (i%2 ==0) {
         opt.synonyms.insert(std::make_pair(std::string("ei_")+std::to_string( i ), true));
         opt.synonyms.insert(std::make_pair(std::string("eint_")+std::to_string( i ), false));
      }
      if (i==1)
         opt.hidden = true;
   }

   gmsopt.setGroup("SecondGroup");
   for( int i = 1; i < 4; ++i )
   {
      GamsOption::EnumVals enumstr;
      for( int j = 0; j < i*2; ++j)
          enumstr.append( std::string("str")+std::to_string(i)+std::to_string(j+1), std::string("enumstr_")+std::to_string(i)+std::string("_")+std::to_string(j) );
      std::string defaultdescr("default description");
      gmsopt.collect(
            std::string("EnumStr_")+std::to_string( i ),
            std::string("description for option EnumStr_")+std::to_string( i ), std::string(),
            std::string("str")+std::to_string(i)+std::to_string(i),  enumstr, defaultdescr);
   }

   for( int i = 0; i < 10; ++i )
   {
      double minval = ((double)1e-1)-i;
      double maxval = (i>5 ? (double)(1e+299) : (double)(2e+4/7.0) * (i+1));
      double f = (double)rand() / RAND_MAX;
      double defaultval = minval + f * (maxval - minval);
      std::string defaultdescr("default description");
      GamsOption& opt(gmsopt.collect(
            std::string("double_")+std::to_string( i ),
            std::string("description for option double_")+std::to_string( i ), std::string(),
            defaultval, minval, maxval, true, true, std::string()) );
      if (i==5)
         opt.synonyms.insert(std::make_pair(std::string("d_")+std::to_string( i ), true));
      else
         opt.synonyms.insert(std::make_pair(std::string("d_")+std::to_string( i ), false));

   }

   gmsopt.setGroup("ThirdGroup");
   for( int i = 0; i < 5; ++i )
   {
      GamsOption& opt(gmsopt.collect(
            std::string("str_")+std::to_string( i ),
            std::string("description for option str_")+std::to_string( i ), std::string(),
            std::string("defval_")+std::to_string( i )) );
      if (i%2 ==0) {
          opt.synonyms.insert(std::make_pair(std::string("s_")+std::to_string( i ), true));
      } else {
          opt.synonyms.insert(std::make_pair(std::string("s_")+std::to_string( i ), false));
      }
   }
   gmsopt.collect( std::string("str_5"),
            std::string("description for option str_5"), std::string());
   gmsopt.collect( std::string("str_6"),
            std::string("description for option str_6"), std::string());

   for( int i = 5; i < 10; ++i )
   {
      int defaultval = i*10;
      int minval = i;
      int maxval = 10000 * i;

      gmsopt.collect(
            std::string("int_")+std::to_string( i ),
            std::string("description for option int_")+std::to_string( i ), std::string(),
            defaultval, minval, maxval, std::string());
   }


   gmsopt.finalize();

   gmsopt.writeDef();
}
