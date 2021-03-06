//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <string.h>

#include "utl/UtlString.h"
#include "utl/XmlContent.h"
#include "xmlparser/tinyxml.h"
#include <sipxunit/TestUtilities.h>

using namespace std ; 

class XmlContentTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(XmlContentTest);
    CPPUNIT_TEST(testNamedEnt);
    CPPUNIT_TEST(testNumericEnt);
    CPPUNIT_TEST(testUnNamedEnt);
    CPPUNIT_TEST(testUnNumericEnt);
    CPPUNIT_TEST(testStringNoNewline);
    CPPUNIT_TEST(testDecimal);
    CPPUNIT_TEST_SUITE_END();

public:

#define ESCAPE(input,correct)                            \
    CPPUNIT_ASSERT_MESSAGE("XmlEscape failed\n"          \
                           "  Input:     '" input "'\n"    \
                           "  Should be: '" correct "'\n", \
                           escape(input,correct)         \
                            );

#define UNESCAPE(input,correct)                            \
    CPPUNIT_ASSERT_MESSAGE("XmlUnEscape failed\n"          \
                           "  Input:     '" input "'\n"    \
                           "  Should be: '" correct "'\n", \
                           unescape(input,correct)         \
                            );


#define HANDCHECK 0 /* unconditionally print results */

   UtlBoolean escape(const char* input, const char* correct)
      {
         UtlString in(input);
         UtlString out;

         XmlEscape(out,in);
         UtlBoolean result = (out == correct);
         if (!result || HANDCHECK)
         {
            printf("---- Escape\n"
                   "  In:  '%s'\n"
                   "  Good:'%s'\n"
                   "  Out: '%s'\n",
                   in.data(), correct, out.data()
                   );
         }
         return result;
      }
      
   UtlBoolean unescape(const char* input, const char* correct)
      {
         UtlString in(input);
         UtlString out;

         XmlUnEscape(out,in);
         UtlBoolean result = (out == correct);
         if (!result || HANDCHECK)
         {
            printf("---- UnEscape\n"
                   "  In:  '%s'\n"
                   "  Good:'%s'\n"
                   "  Out: '%s'\n",
                   in.data(), correct, out.data()
                   );
         }
         return result;
      }
      
   void testNamedEnt()
      {
         ESCAPE("zigzag","zigzag");
         ESCAPE("zig&zag","zig&amp;zag");
         ESCAPE("zig&&zag","zig&amp;&amp;zag");
         ESCAPE("zigzag&","zigzag&amp;");
         ESCAPE("zigzag&&","zigzag&amp;&amp;");
         ESCAPE("&zigzag","&amp;zigzag");
         ESCAPE("&&zigzag","&amp;&amp;zigzag");
         ESCAPE("&zigzag&","&amp;zigzag&amp;");
         ESCAPE("&&zigzag&&","&amp;&amp;zigzag&amp;&amp;");
         ESCAPE("&zig&zag&","&amp;zig&amp;zag&amp;");
         ESCAPE("&&zig&&zag&&","&amp;&amp;zig&amp;&amp;zag&amp;&amp;");
      }

   void testNumericEnt()
      {
         ESCAPE("zigzag","zigzag");
         ESCAPE("zig\x03zag","zig&#x03;zag");
         ESCAPE("zig\x03\x03zag","zig&#x03;&#x03;zag");
         ESCAPE("zigzag\x03","zigzag&#x03;");
         ESCAPE("zigzag\x03\x03","zigzag&#x03;&#x03;");
         ESCAPE("\x03zigzag","&#x03;zigzag");
         ESCAPE("\x03\x03zigzag","&#x03;&#x03;zigzag");
         ESCAPE("\x03zigzag\x03","&#x03;zigzag&#x03;");
         ESCAPE("\x03\x03zigzag\x03\x03","&#x03;&#x03;zigzag&#x03;&#x03;");
         ESCAPE("\x03zig\x03zag\x03","&#x03;zig&#x03;zag&#x03;");
         ESCAPE("\x03\x03zig\x03\x03zag\x03\x03","&#x03;&#x03;zig&#x03;&#x03;zag&#x03;&#x03;");
      }

   void testUnNamedEnt()
      {
         UNESCAPE("zigzag","zigzag");
         UNESCAPE("zig&amp;zag","zig&zag");
         UNESCAPE("zig&amp;&amp;zag","zig&&zag");
         UNESCAPE("zigzag&amp;","zigzag&");
         UNESCAPE("zigzag&amp;&amp;","zigzag&&");
         UNESCAPE("&amp;zigzag","&zigzag");
         UNESCAPE("&amp;&amp;zigzag","&&zigzag");
         UNESCAPE("&amp;zigzag&amp;","&zigzag&");
         UNESCAPE("&amp;&amp;zigzag&amp;&amp;","&&zigzag&&");
         UNESCAPE("&amp;zig&amp;zag&amp;","&zig&zag&");
         UNESCAPE("&amp;&amp;zig&amp;&amp;zag&amp;&amp;","&&zig&&zag&&");
      }

   void testUnNumericEnt()
      {
         UNESCAPE("zigzag","zigzag");
         UNESCAPE("zig&#x03;zag","zig\x03zag");
         UNESCAPE("zig&#x03;&#x03;zag","zig\x03\x03zag");
         UNESCAPE("zigzag&#x03;","zigzag\x03");
         UNESCAPE("zigzag&#x03;&#x03;","zigzag\x03\x03");
         UNESCAPE("&#x03;zigzag","\x03zigzag");
         UNESCAPE("&#x03;&#x03;zigzag","\x03\x03zigzag");
         UNESCAPE("&#x03;zigzag&#x03;","\x03zigzag\x03");
         UNESCAPE("&#x03;&#x03;zigzag&#x03;&#x03;","\x03\x03zigzag\x03\x03");
         UNESCAPE("&#x03;zig&#x03;zag&#x03;","\x03zig\x03zag\x03");
         UNESCAPE("&#x03;&#x03;zig&#x03;&#x03;zag&#x03;&#x03;","\x03\x03zig\x03\x03zag\x03\x03");
      }

   // Test that TiXmlDocument::Parse works on a string that does not end with newline.
   void testStringNoNewline()
      {
         const char* test_string = "<document-info>\n</document-info>";

         TiXmlDocument doc;

         const char* result = doc.Parse(test_string);

         CPPUNIT_ASSERT_MESSAGE("Parse of test_string failed.", result != 0);
      }

   void testDecimal()
      {
         UtlString s;
         s = "";
         XmlDecimal(s, 1);
         CPPUNIT_ASSERT_EQUAL(s, UtlString("1"));
         XmlDecimal(s, -1);
         CPPUNIT_ASSERT_EQUAL(s, UtlString("1-1"));
         XmlDecimal(s, 12);
         CPPUNIT_ASSERT_EQUAL(s, UtlString("1-112"));
         XmlDecimal(s, 1234567890);
         CPPUNIT_ASSERT_EQUAL(s, UtlString("1-1121234567890"));
         s = "";
         XmlDecimal(s, 1, "%020d");
         CPPUNIT_ASSERT_EQUAL(s, UtlString("00000000000000000001"));
         s = "";
         XmlDecimal(s, (unsigned int) 0xFFFFFFFF, "%u");
         CPPUNIT_ASSERT_EQUAL(s, UtlString("4294967295"));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(XmlContentTest);
