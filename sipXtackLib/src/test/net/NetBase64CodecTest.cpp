//
// Copyright (C) 2004, 2005 Pingtel Corp.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/NetBase64Codec.h>

typedef struct
{
   size_t      inputSize;
   const char* inputData;
   const char* output;
} TestData;

// Table of test cases.
TestData tests[] =
{
   {
      24,
      "Base64 encode or decode\n",
      "QmFzZTY0IGVuY29kZSBvciBkZWNvZGUK"
   },
   {
      25,
      "Base64 encode or decodeX\n",
      "QmFzZTY0IGVuY29kZSBvciBkZWNvZGVYCg=="
   },
   {
      26,
      "Base64 encode or decodeXX\n",
      "QmFzZTY0IGVuY29kZSBvciBkZWNvZGVYWAo="
   },
   {
      25,
      "Base64 encode or \000decode\n",
      "QmFzZTY0IGVuY29kZSBvciAAZGVjb2RlCg=="
   }
};

/**
 * Unittest for NetBase64Codec
 */
class NetBase64CodecTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(NetBase64CodecTest);
   CPPUNIT_TEST(testIsValid);
   CPPUNIT_TEST(testDecodedSize);
   CPPUNIT_TEST(testChar2Char);
   CPPUNIT_TEST(testChar2Utl);
   CPPUNIT_TEST(testUtl2Utl);
   CPPUNIT_TEST(testBadDecodeSize);
   CPPUNIT_TEST(testBadDecode);
   CPPUNIT_TEST_SUITE_END();


public:

   
   void testIsValid()
      {
         UtlString notBase64;

         notBase64 = "$$%%"; // invalid contents
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "$$%%^^=="; // invalid contents with valid length padding
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "12345"; // not an integral number of encoded bytes
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "123456"; // not enough padding characters - should end "=="
                        CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "1234567"; // not enough padding characters - should end "="
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "12345=="; // not an integral number of encoded bytes
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "123456="; // not enough padding characters - should end "=="
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "1234567=="; // to many padding characters - should end "="
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "123456==="; // not enough padding characters - should end "=="
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "1234567==="; // to many padding characters - should end "="
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));

         notBase64 = "123456===7"; // padding characters not at the end
         CPPUNIT_ASSERT(! NetBase64Codec::isValid(notBase64));
      }
   
   
   void testBadDecodeSize()
      {
         UtlString notBase64;

         notBase64 = "$$%^^";

         int decodedSize;

         decodedSize = NetBase64Codec::decodedSize(notBase64);
         CPPUNIT_ASSERT_EQUAL(0, decodedSize);

         notBase64 = "$$%^";

         decodedSize = NetBase64Codec::decodedSize(notBase64);
         CPPUNIT_ASSERT_EQUAL(0, decodedSize);
         
      }
   
   void testBadDecode()
      {
         UtlString notBase64;

         notBase64 = "$$%^^";

         UtlString decoded;
         CPPUNIT_ASSERT(! NetBase64Codec::decode(notBase64, decoded));

         CPPUNIT_ASSERT(decoded.isNull());
      }
   
   void testDecodedSize()
      {
         int  decodedSize;
         char msg[2048];

         for (unsigned int test = 0; test < (sizeof(tests)/sizeof(TestData)); test++)
         {
            decodedSize = NetBase64Codec::decodedSize(strlen(tests[test].output),
                                                      tests[test].output
                                                      );

            sprintf(msg,
                    "\n  test case %d decodedSize"
                    "\n     expected %d"
                    "\n     actual   %d",
                    test, tests[test].inputSize, decodedSize
                    );
            CPPUNIT_ASSERT_MESSAGE(msg, decodedSize == (int)tests[test].inputSize);
         }
      }


   void testChar2Char()
      {
         int  encodedSize;
         char encodedData[1024];
         int  decodedSize;
         char decodedData[1024];
         char msg[2048];

         for (unsigned int test = 0; test < (sizeof(tests)/sizeof(TestData)); test++)
         {            
            encodedData[0] = 0; // to make message cleaner below if not written
            NetBase64Codec::encode(tests[test].inputSize,
                                   tests[test].inputData,
                                   encodedSize,
                                   encodedData
                                   );
            encodedData[encodedSize] = '\000';
            
            sprintf(msg,
                    "\n  test case %d encoding"
                    "\n     expected size %d data '%s'"
                    "\n     actual   size %d data '%s'",
                    test,
                    strlen(tests[test].output), tests[test].output,
                    encodedSize, encodedData
                    );
            CPPUNIT_ASSERT_MESSAGE(msg,
                                   (   (encodedSize == (int)strlen(tests[test].output))
                                    && (strcmp(tests[test].output, encodedData) == 0)
                                    ));

            decodedData[0] = '\000'; // to make message cleaner below if not written
            bool decodedOk;
            decodedOk = NetBase64Codec::decode(encodedSize,
                                               encodedData,
                                               decodedSize,
                                               decodedData
                                               );

            decodedData[decodedSize] = '\000';
            sprintf(msg,
                    "\n  test case %d decoding %s"
                    "\n     expected size %d data '%s'"
                    "\n     actual   size %d data '%s'",
                    test, decodedOk ? "ok" : "failed",
                    tests[test].inputSize, tests[test].inputData,
                    decodedSize, decodedData
                    );
            CPPUNIT_ASSERT_MESSAGE(msg,
                                   (   decodedOk
                                    && (decodedSize == (int)tests[test].inputSize)
                                    && (memcmp(tests[test].inputData,
                                               decodedData,
                                               tests[test].inputSize
                                               )
                                        == 0)
                                    ));

         }
      }

   void testChar2Utl()
      {
         UtlString encoded;

         char msg[2048];

         for (unsigned int test = 0; test < (sizeof(tests)/sizeof(TestData)); test++)
         {
            encoded.remove(0);
            NetBase64Codec::encode(tests[test].inputSize,
                                   tests[test].inputData,
                                   encoded
                                   );
            
            sprintf(msg,
                    "\n  test case %d encoding"
                    "\n     expected size %d data '%s'"
                    "\n     actual   size %d data '%s'",
                    test,
                    strlen(tests[test].output), tests[test].output,
                    encoded.length(), encoded.data()
                    );
            CPPUNIT_ASSERT_MESSAGE(msg,
                                   (   (encoded.length() == strlen(tests[test].output))
                                    && (encoded.compareTo(tests[test].output) == 0)
                                    ));
         }
      }

   
   void testUtl2Utl()
      {
         UtlString encoded;
         UtlString decoded;

         char msg[2048];

         for (unsigned int test = 0; test < (sizeof(tests)/sizeof(TestData)); test++)
         {
            UtlString input(tests[test].inputData, tests[test].inputSize);
            
            encoded.remove(0);
            NetBase64Codec::encode(input, encoded);
            
            sprintf(msg,
                    "\n  test case %d encoding"
                    "\n     expected size %d data '%s'"
                    "\n     actual   size %d data '%s'",
                    test,
                    strlen(tests[test].output), tests[test].output,
                    encoded.length(), encoded.data()
                    );
            CPPUNIT_ASSERT_MESSAGE(msg,
                                   (   (encoded.length() == strlen(tests[test].output))
                                    && (encoded.compareTo(tests[test].output) == 0)
                                    ));

            decoded.remove(0);
            bool decodedOk;
            decodedOk = NetBase64Codec::decode(encoded, decoded);

            sprintf(msg,
                    "\n  test case %d decoding %s"
                    "\n     expected size %d data '%s'"
                    "\n     actual   size %d data '%s'",
                    test, decodedOk ? "ok" : "failed",
                    tests[test].inputSize, tests[test].inputData,
                    decoded.length(), decoded.data()
                    );
            CPPUNIT_ASSERT_MESSAGE(msg,
                                   (   decodedOk
                                    && (decoded.length() == tests[test].inputSize)
                                    && (memcmp(tests[test].inputData,
                                               decoded.data(),
                                               tests[test].inputSize
                                               )
                                        == 0)
                                    ));
         }
      }

};




CPPUNIT_TEST_SUITE_REGISTRATION(NetBase64CodecTest);
