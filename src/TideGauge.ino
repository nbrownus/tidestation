/*
 * Project TideStation
 * Description: Sentient Things Tide Station
 * Author: Robert Mawrey
 * Date: April 2020
 * Version 1.8
 * Corrected bug in setting us units
 * Version 1.7
 * Update TideStats to 0.0.4 to use Time.now()
 * Version 1.6
 * Error message edits
 * Fix bug with hardreset delay hard fault
 * Version 1.5
 * Added explicit hard reset messages
 * Version 1.4
 * Update to only use the Particle time as the device is always on.
 */
#include "IoTNode.h"
#include "TideStats.h"
#include "IoTNodeThingSpeak.h"
#include "WeatherLevel.h"
#include "SdCardLogHandlerRK.h"
// Create a thingspeakkeys.h file to include
// or comment out this include
#include "thingspeakkeys.h"

/*
//Contents of thingspeakkeys.h file
#define THINGSPEAK_KEYS
// Get these parameters from your ThingSpeak Channel
#define THINGSPEAK_CHANNEL_ID "XXXXXXX"
#define THINGSPEAK_WRITE_KEY "XXXXXXXXXXXXXXXXX"
*/


SYSTEM_THREAD(ENABLED);
// SYSTEM_MODE(MANUAL);

#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
  #define DEBUG_PRINTLNF(...) Serial.printlnf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_PRINTF(...)
  #define DEBUG_PRINTLNF(...)
#endif

// Comment out as appropriate for default units
#define UNITS_US
//#define UNITS_METRIC

// THINGSPEAK_KEYS is defined in thingspeakkeys.h if it exists
#ifndef THINGSPEAK_KEYS
// Get these parameters from your ThingSpeak Channel
#define THINGSPEAK_CHANNEL_ID "XXXXXXX"
#define THINGSPEAK_WRITE_KEY "XXXXXXXXXXXXXXXXX"
#endif

String readings;
String maxbotixranges = "unknown";
uint32_t syncTime = 1584569584; // time of boot sync or 03/18/2020 @ 10:13pm (UTC)
String mllwCalibrationMess = "unknown"; // Used to display hours left for mllw calibration

const int SD_CHIP_SELECT = N_D0;
SdFat sd;
SdCardPrintHandler printToSd(sd, SD_CHIP_SELECT, SPI_FULL_SPEED);


STARTUP(printToSd.withMaxFilesToKeep(3000));

#define SENSOR_READ_RATE_MA 6000 
Timer readSensorTimer(SENSOR_READ_RATE_MA, readSensors);

#define SENSOR_SEND_RATE_MA 60000 // Standard 60000
Timer sendSensorTimer(SENSOR_SEND_RATE_MA, sendSensors);

// #define SEND_TEST_DATA

int testNum=0;
#ifdef SEND_TEST_DATA
static const int testcount = 767;
static const uint32_t testTime[]={1569888000,1569891600,1569895200,1569898800,1569902400,1569906000,1569909600,1569913200,1569916800,1569920400,1569924000,1569927600,1569931200,1569934800,1569938400,1569942000,1569945600,1569949200,1569952800,1569956400,1569960000,1569963600,1569967200,1569970800,1569974400,1569978000,1569981600,1569985200,1569988800,1569992400,1569996000,1569999600,1570003200,1570006800,1570010400,1570014000,1570017600,1570021200,1570024800,1570028400,1570032000,1570035600,1570039200,1570042800,1570046400,1570050000,1570053600,1570057200,1570060800,1570064400,1570068000,1570071600,1570075200,1570078800,1570082400,1570086000,1570089600,1570093200,1570096800,1570100400,1570104000,1570107600,1570111200,1570114800,1570118400,1570122000,1570125600,1570129200,1570132800,1570136400,1570140000,1570143600,1570147200,1570150800,1570154400,1570158000,1570161600,1570165200,1570168800,1570172400,1570176000,1570179600,1570183200,1570186800,1570190400,1570194000,1570197600,1570201200,1570204800,1570208400,1570212000,1570215600,1570219200,1570222800,1570226400,1570230000,1570233600,1570237200,1570240800,1570244400,1570248000,1570251600,1570255200,1570258800,1570262400,1570266000,1570269600,1570273200,1570276800,1570280400,1570284000,1570287600,1570291200,1570294800,1570298400,1570302000,1570305600,1570309200,1570312800,1570316400,1570320000,1570323600,1570327200,1570330800,1570334400,1570338000,1570341600,1570345200,1570348800,1570352400,1570356000,1570359600,1570363200,1570366800,1570370400,1570374000,1570377600,1570381200,1570384800,1570388400,1570392000,1570395600,1570399200,1570402800,1570406400,1570410000,1570413600,1570417200,1570420800,1570424400,1570428000,1570431600,1570435200,1570438800,1570442400,1570446000,1570449600,1570453200,1570456800,1570460400,1570464000,1570467600,1570471200,1570474800,1570478400,1570482000,1570485600,1570489200,1570492800,1570496400,1570500000,1570503600,1570507200,1570510800,1570514400,1570518000,1570521600,1570525200,1570528800,1570532400,1570536000,1570539600,1570543200,1570546800,1570550400,1570554000,1570557600,1570561200,1570564800,1570568400,1570572000,1570575600,1570579200,1570582800,1570586400,1570590000,1570593600,1570597200,1570600800,1570604400,1570608000,1570611600,1570615200,1570618800,1570622400,1570626000,1570629600,1570633200,1570636800,1570640400,1570644000,1570647600,1570651200,1570654800,1570658400,1570662000,1570665600,1570669200,1570672800,1570676400,1570680000,1570683600,1570687200,1570690800,1570694400,1570698000,1570701600,1570705200,1570708800,1570712400,1570716000,1570719600,1570723200,1570726800,1570730400,1570734000,1570737600,1570741200,1570744800,1570748400,1570752000,1570755600,1570759200,1570762800,1570766400,1570770000,1570773600,1570777200,1570780800,1570784400,1570788000,1570791600,1570795200,1570798800,1570802400,1570806000,1570809600,1570813200,1570816800,1570820400,1570824000,1570827600,1570831200,1570834800,1570838400,1570842000,1570845600,1570849200,1570852800,1570856400,1570860000,1570863600,1570867200,1570870800,1570874400,1570878000,1570881600,1570885200,1570888800,1570892400,1570896000,1570899600,1570903200,1570906800,1570910400,1570914000,1570917600,1570921200,1570924800,1570928400,1570932000,1570935600,1570939200,1570942800,1570946400,1570950000,1570953600,1570957200,1570960800,1570964400,1570968000,1570971600,1570975200,1570978800,1570982400,1570986000,1570989600,1570993200,1570996800,1571000400,1571004000,1571007600,1571011200,1571014800,1571018400,1571022000,1571025600,1571029200,1571032800,1571036400,1571040000,1571043600,1571047200,1571050800,1571054400,1571058000,1571061600,1571065200,1571068800,1571072400,1571076000,1571079600,1571083200,1571086800,1571090400,1571094000,1571097600,1571101200,1571104800,1571108400,1571112000,1571115600,1571119200,1571122800,1571126400,1571130000,1571133600,1571137200,1571140800,1571144400,1571148000,1571151600,1571155200,1571158800,1571162400,1571166000,1571169600,1571173200,1571176800,1571180400,1571184000,1571187600,1571191200,1571194800,1571198400,1571202000,1571205600,1571209200,1571212800,1571216400,1571220000,1571223600,1571227200,1571230800,1571234400,1571238000,1571241600,1571245200,1571248800,1571252400,1571256000,1571259600,1571263200,1571266800,1571270400,1571274000,1571277600,1571281200,1571284800,1571288400,1571292000,1571295600,1571299200,1571302800,1571306400,1571310000,1571313600,1571317200,1571320800,1571324400,1571328000,1571331600,1571335200,1571338800,1571342400,1571346000,1571349600,1571353200,1571356800,1571360400,1571364000,1571367600,1571371200,1571374800,1571378400,1571382000,1571385600,1571389200,1571392800,1571396400,1571400000,1571403600,1571407200,1571410800,1571414400,1571418000,1571421600,1571425200,1571428800,1571432400,1571436000,1571439600,1571443200,1571446800,1571450400,1571454000,1571457600,1571461200,1571464800,1571468400,1571472000,1571475600,1571479200,1571482800,1571486400,1571490000,1571493600,1571497200,1571500800,1571504400,1571508000,1571511600,1571515200,1571518800,1571522400,1571526000,1571529600,1571533200,1571536800,1571540400,1571544000,1571547600,1571551200,1571554800,1571558400,1571562000,1571565600,1571569200,1571572800,1571576400,1571580000,1571583600,1571587200,1571590800,1571594400,1571598000,1571601600,1571605200,1571608800,1571612400,1571616000,1571619600,1571623200,1571626800,1571630400,1571634000,1571637600,1571641200,1571644800,1571648400,1571652000,1571655600,1571659200,1571662800,1571666400,1571670000,1571673600,1571677200,1571680800,1571684400,1571688000,1571691600,1571695200,1571698800,1571702400,1571706000,1571709600,1571713200,1571716800,1571720400,1571724000,1571727600,1571731200,1571734800,1571738400,1571742000,1571745600,1571749200,1571752800,1571756400,1571760000,1571763600,1571767200,1571770800,1571774400,1571778000,1571781600,1571785200,1571788800,1571792400,1571796000,1571799600,1571803200,1571806800,1571810400,1571814000,1571817600,1571821200,1571824800,1571828400,1571832000,1571835600,1571839200,1571842800,1571846400,1571850000,1571853600,1571857200,1571860800,1571864400,1571868000,1571871600,1571875200,1571878800,1571882400,1571886000,1571889600,1571893200,1571896800,1571900400,1571904000,1571907600,1571911200,1571914800,1571918400,1571922000,1571925600,1571929200,1571932800,1571936400,1571940000,1571943600,1571947200,1571950800,1571954400,1571958000,1571961600,1571965200,1571968800,1571972400,1571976000,1571979600,1571983200,1571986800,1571990400,1571994000,1571997600,1572001200,1572004800,1572008400,1572012000,1572015600,1572019200,1572022800,1572026400,1572030000,1572033600,1572037200,1572040800,1572044400,1572048000,1572051600,1572055200,1572058800,1572062400,1572066000,1572069600,1572073200,1572076800,1572080400,1572084000,1572087600,1572091200,1572094800,1572098400,1572102000,1572105600,1572109200,1572112800,1572116400,1572120000,1572123600,1572127200,1572130800,1572134400,1572138000,1572141600,1572145200,1572148800,1572152400,1572156000,1572159600,1572163200,1572166800,1572170400,1572174000,1572177600,1572181200,1572184800,1572188400,1572192000,1572195600,1572199200,1572202800,1572206400,1572210000,1572213600,1572217200,1572220800,1572224400,1572228000,1572231600,1572235200,1572238800,1572242400,1572246000,1572249600,1572253200,1572256800,1572260400,1572264000,1572267600,1572271200,1572274800,1572278400,1572282000,1572285600,1572289200,1572292800,1572296400,1572300000,1572303600,1572307200,1572310800,1572314400,1572318000,1572321600,1572325200,1572328800,1572332400,1572336000,1572339600,1572343200,1572346800,1572350400,1572354000,1572357600,1572361200,1572364800,1572368400,1572372000,1572375600,1572379200,1572382800,1572386400,1572390000,1572393600,1572397200,1572400800,1572404400,1572408000,1572411600,1572415200,1572418800,1572422400,1572426000,1572429600,1572433200,1572436800,1572440400,1572444000,1572447600,1572451200,1572454800,1572458400,1572462000,1572465600,1572469200,1572472800,1572476400,1572480000,1572483600,1572487200,1572490800,1572494400,1572498000,1572501600,1572505200,1572508800,1572512400,1572516000,1572519600,1572523200,1572526800,1572530400,1572534000,1572537600,1572541200,1572544800,1572548400,1572552000,1572555600,1572559200,1572562800,1572566400,1572570000,1572573600,1572577200,1572580800,1572584400,1572588000,1572591600,1572595200,1572598800,1572602400,1572606000,1572609600,1572613200,1572616800,1572620400,1572624000,1572627600,1572631200,1572634800,1572638400,1572642000,1572645600};
static const float range[] = {2622,2629,2461,2243,2043,1878,1689,1601,1699,1858,2077,2318,2519,2576,2387,2150,1934,1753,1574,1432,1518,1689,1909,2183,2426,2603,2605,2434,2233,2060,1912,1731,1670,1769,1922,2135,2350,2530,2537,2352,2143,1958,1779,1578,1455,1532,1705,1917,2172,2413,2576,2555,2375,2187,2020,1897,1732,1642,1742,1847,2030,2222,2333,2277,2107,1927,1771,1651,1532,1455,1556,1680,1913,2168,2391,2545,2469,2314,2173,2024,1875,1693,1686,1774,1888,2090,2279,2453,2507,2361,2181,2022,1837,1647,1554,1629,1775,1979,2236,2457,2597,2598,2436,2282,2143,1971,1828,1845,1911,2002,2150,2312,2430,2348,2195,2057,1974,1860,1698,1626,1705,1845,2036,2264,2462,2571,2503,2397,2314,2209,2062,1931,1862,1882,1952,2107,2269,2355,2291,2156,2065,1971,1848,1683,1632,1720,1835,2001,2213,2416,2553,2528,2427,2332,2238,2105,1936,1843,1897,2021,2174,2339,2456,2436,2321,2214,2087,1952,1790,1711,1736,1865,2024,2234,2437,2582,2610,2505,2375,2261,2097,1930,1842,1948,2045,2183,2338,2438,2404,2290,2158,2034,1898,1755,1690,1743,1815,1975,2139,2303,2392,2287,2177,2067,1963,1818,1698,1697,1791,1925,2055,2210,2301,2259,2083,1901,1794,1699,1633,1634,1682,1778,1930,2061,2199,2230,2115,2005,1942,1797,1643,1624,1627,1695,1819,1946,2054,2060,1927,1777,1729,1705,1579,1499,1501,1567,1680,1843,1993,2110,2008,1853,1723,1639,1539,1460,1436,1532,1602,1751,1901,1979,1917,1778,1664,1576,1562,1502,1474,1549,1645,1775,1934,2052,2044,1941,1845,1766,1680,1629,1595,1655,1745,1876,2061,2223,2316,2189,2040,1946,1891,1832,1713,1652,1753,1865,2026,2196,2303,2290,2156,2038,1953,1876,1795,1703,1733,1834,1987,2182,2345,2363,2240,2111,2037,1941,1841,1714,1710,1798,1931,2096,2236,2323,2238,2079,1961,1869,1773,1638,1588,1704,1845,2017,2217,2371,2380,2247,2140,2048,1955,1826,1723,1760,1861,2004,2181,2361,2413,2261,2111,1990,1912,1808,1685,1720,1828,1977,2165,2354,2492,2438,2301,2183,2087,1970,1817,1762,1834,1948,2099,2257,2341,2257,2093,1960,1840,1720,1570,1507,1576,1699,1877,2071,2206,2205,2036,1810,1605,1361,1033,866,1143,1392,1686,1989,2187,2278,2146,2041,1973,1895,1829,1693,1702,1815,2012,2268,2494,2615,2624,2501,2353,2231,2129,2001,1904,1915,2034,2175,2320,2417,2391,2262,2112,1969,1837,1675,1581,1651,1739,1905,2115,2337,2518,2475,2345,2236,2149,2023,1890,1841,1934,2049,2187,2301,2380,2322,2192,2066,1961,1834,1694,1638,1700,1827,2019,2235,2445,2577,2492,2359,2247,2151,2009,1858,1799,1886,1969,2103,2246,2342,2265,2137,2035,1921,1790,1638,1582,1674,1775,1988,2219,2407,2509,2406,2294,2204,2105,1975,1822,1760,1818,1936,2103,2268,2317,2201,2045,1956,1857,1706,1554,1487,1582,1747,1983,2223,2430,2534,2443,2318,2200,2045,1899,1748,1739,1752,1909,2083,2249,2309,2211,2062,1940,1797,1646,1468,1394,1499,1638,1837,2067,2257,2338,2235,2115,1979,1848,1717,1612,1560,1658,1825,2027,2245,2390,2299,2143,1976,1806,1701,1620,1612,1690,1830,2045,2289,2495,2601,2539,2378,2229,2077,1933,1803,1790,1919,2056,2271,2469,2602,2604,2460,2282,2099,1937,1786,1693,1763,1910,2120,2353,2556,2629,2601,2427,2245,2073,1908,1760,1735,1865,2044,2265,2466,2610,2625,2484,2290,2102,1946,1761,1642,1713,1874,2076,2316,2534,2619,2519,2313,2116,1969,1808,1661,1675,1810,1985,2215,2448,2609,2620,2454,2240,2049,1884,1699,1610,1702,1868,2088,2322,2525,2588,2400,2155,1918,1714,1523,1366,1401,1489,1664,1871,2051,2110,2007,1887,1813,1785,1816,1708,1635,1765,1953,2178,2401,2581,2605,2401,2152,1914,1692,1478,1367,1458,1602,1821,2091,2338,2511,2418,2234,2057,1928,1819,1665,1637,1746,1919,2134,2348,2505,2450,2244,2010,1821,1696,1566,1474,1562,1741,1982,2248,2477,2609,2572,2393,2213,2059,1899,1713,1683,1794,1956,2157,2364,2516,2419,2211,1999,1817,1677,1509,1461,1592,1775,2014,2277,2501,2612,2541,2367,2188,2031,1866,1692,1674,1781,1946,2145,2309,2376,2225,2034,1868,1716,1581,1420,1385,1511,1682,1922,2176,2397,2490,2366,2196,2023,1837,1668,1508,1474,1613,1829,2156,2428,2602,2572,2378,2249,2115,1902,1723,1704,2518};
#endif

// EXAMPLE - defining and using a LED status
LEDStatus blinkYellow(RGB_COLOR_YELLOW, LED_PATTERN_BLINK, LED_SPEED_FAST, LED_PRIORITY_IMPORTANT);

LEDStatus blinkRed(RGB_COLOR_RED, LED_PATTERN_BLINK, LED_SPEED_FAST, LED_PRIORITY_IMPORTANT);

LEDStatus blinkBlue(RGB_COLOR_BLUE, LED_PATTERN_BLINK, LED_SPEED_FAST, LED_PRIORITY_IMPORTANT);

LEDStatus blinkOrange(RGB_COLOR_ORANGE, LED_PATTERN_BLINK, LED_SPEED_FAST, LED_PRIORITY_IMPORTANT);

IoTNode node;

TideStats tide(node);

Maxbotix maxbotix(node, 99);

Weather weatherSensors;

typedef struct
{
    bool fahrenheit; // C
    int firstrun;  
    float speedfactor; // m/s
    float levelfactor;
    float pressurefactor; // millibars
}TSunit_t;

TSunit_t tsunit;

int firstrunvalue = 123456;

String deviceString = "Devices = ";

// Create an array of tsunit struct
framArray framtsunit = node.makeFramArray(1, sizeof(tsunit));


// This Webhook must be created using your Particle account
// See https://docs.particle.io/tutorials/device-cloud/webhooks/#custom-template
// {
//     "event": "TSBulkWriteCSV",
//     "responseTopic": "{{PARTICLE_DEVICE_ID}}/hook-response/TSBulkWriteCSV",
//     "url": "https://api.thingspeak.com/channels/{{c}}/bulk_update.csv",
//     "requestType": "POST",
//     "noDefaults": true,
//     "rejectUnauthorized": true,
//     "responseTemplate": "{{success}}",
//     "headers": {
//         "Content-Type": "application/x-www-form-urlencoded"
//     },
//     "form": {
//         "write_api_key": "{{k}}",
//         "time_format": "{{t}}",
//         "updates": "{{d}}"
//     }
// }

IoTNodeThingSpeak thingSpeak(node, THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_KEY, 300);

/**
 * @brief This is the necessary struct format for ThingSpeak data
 * It matches the ThingSpeal Bulk-Write CSV Data format: https://www.mathworks.com/help/thingspeak/bulkwritecsvdata.html
 * TIMESTAMP,FIELD1_VALUE,FIELD2_VALUE,FIELD3_VALUE,FIELD4_VALUE,FIELD5_VALUE,FIELD6_VALUE,FIELD7_VALUE,FIELD8_VALUE,LATITUDE,LONGITUDE,ELEVATION,STATUS
 * and is queued in the IoTNode Fram
 * nullMap is used as a bitmap to send nulls for bits that are equal to 1
 */
typedef struct
{
  uint32_t messageTime;
  float field1; // level relative to mllw
  float field2; // wind speed
  float field3; // wind direction (degrees)
  float field4; // water temp
  float field5; // air temp
  float field6; // humidity
  float field7; // pressure
  float field8; // battery charge or voltage
  float latitude; // null
  float longitude; // null
  float elevation; // null
  char status[48];
  uint16_t nullMap; // bits define null values i.e. msb=field1, bit 0 = status
}TSdata_t;

TSdata_t tsdata;

void readSensors()
{
  if (!checkI2CDevices())
  {
    DEBUG_PRINTLN("Error reading I2C devices. Resetting.");
    if (Particle.connected())
    {
      Particle.publish("Error reading I2C devices. Resetting.",PRIVATE);
    }
    delay(500);
    System.reset();
  }
  
  weatherSensors.captureWindVane();
  weatherSensors.captureTempHumidityPressure();
  weatherSensors.captureWaterTemp();
  weatherSensors.captureBatteryVoltage();
  int rangeup;
  uint32_t readingTime;
  #ifdef SEND_TEST_DATA
  if (testNum>=testcount)
  {
    testNum = 0;
  }
  readingTime = testTime[testNum];
  #else
  // Use Particle time since the device is always on and the RTC time is not needed
  readingTime = Time.now();

  // Check to make sure that the time has not reset somehow
  if (!(readingTime >= syncTime))
  {
    if (Particle.connected())
    {
      Particle.publish("Resetting because time NOT synced:",String(Time.format(readingTime, TIME_FORMAT_ISO8601_FULL)),PRIVATE);
    }
    DEBUG_PRINT("Resetting because time NOT synced: ");
    DEBUG_PRINTLN(String(Time.format(readingTime, TIME_FORMAT_ISO8601_FULL)));
    delay(100);
    System.reset();
  }
  #endif  

  if (maxbotix.dualSensor)
  {
    rangeup = -maxbotix.range1Dual();
    maxbotixranges = String("Range 1:"+String(maxbotix.range1Median())+", "+"Range 2:"+String(maxbotix.range2Median()));
  }
  else
  {
    rangeup = -maxbotix.range1Median();
    maxbotixranges = String("Range 1:"+String(maxbotix.range1Median()));
  }

  #ifdef SEND_TEST_DATA
  rangeup = -range[testNum];
  Time.setTime(readingTime);
  #endif

  if (rangeup<0)
  {
    tide.pushDistanceUpwards(rangeup,readingTime);
    DEBUG_PRINTF("Maxbotix range is %d mm at ",rangeup);
    DEBUG_PRINTLN(String(Time.format(readingTime, TIME_FORMAT_ISO8601_FULL)));    
  }
  else
  {
    DEBUG_PRINT("Invalid Maxbotix readings: ");
    DEBUG_PRINTLN(maxbotixranges);
  }

}

void sendSensors()
{
  int rangeup;

  uint32_t readingTime;
  #ifdef SEND_TEST_DATA
  if (testNum>=testcount)
  {
    testNum = 0;
  }
  readingTime = testTime[testNum];
  #else
  // Use Particle time since the device is always on and the RTC time is not needed
  readingTime = Time.now();

  // Check to make sure that the time has not reset somehow
  if (!(readingTime >= syncTime))
  {
    if (Particle.connected())
    {
      Particle.publish("Resetting because time NOT synced:",String(Time.format(readingTime, TIME_FORMAT_ISO8601_FULL)),PRIVATE);
    }
    DEBUG_PRINT("Resetting because time NOT synced: ");
    DEBUG_PRINTLN(String(Time.format(readingTime, TIME_FORMAT_ISO8601_FULL)));
    delay(100);
    System.reset();
  }
  #endif

  if (maxbotix.dualSensor)
  {
    rangeup = -maxbotix.range1Dual();
  }
  else
  {
    rangeup = -maxbotix.range1Median();
  }

  #ifdef SEND_TEST_DATA
  rangeup = -range[testNum];
  testNum++;
  Time.setTime(readingTime);
  #endif
  tsdata.messageTime = readingTime;
  DEBUG_PRINTF("Maxbotix range is %d mm at ",rangeup);
  DEBUG_PRINTLN(String(Time.format(readingTime, TIME_FORMAT_ISO8601_FULL)));
  
  if (rangeup<0)
  {
    tsdata.field1 = (rangeup - tide.mllw())*tsunit.levelfactor;
    // Send tide values
    tsdata.nullMap = tsdata.nullMap & 0B011111111111;                                      
  }
  else
  {
    // Send null for field 1
    tsdata.nullMap = tsdata.nullMap | 0B100000000000;
  }
  if (tide.mllw()==0)
  {
    // Then mllw not yet calculated so don't send the tide values to ThingSpeak
    tsdata.nullMap = tsdata.nullMap | 0B100000000000;
    mllwCalibrationMess = String::format("%.2f hours until mllw calibrated and tide published", tide.mllwCalibrationHoursLeft());
    if (Particle.connected())
    {
      Particle.publish("Calibrating",mllwCalibrationMess,PRIVATE);
    }
    DEBUG_PRINTLN(mllwCalibrationMess);
  }
  else
  {
    mllwCalibrationMess = "mllw calibrated";
  }
  float gustMPH;
  tsdata.field2 = weatherSensors.getAndResetAnemometerMPH(&gustMPH);
  tsdata.field3 = (float)weatherSensors.getAndResetWindVaneDegrees();
  float waterTempF = weatherSensors.getAndResetWaterTempF();
  if (waterTempF < 5.0)
  {
    // Send null
    tsdata.nullMap = tsdata.nullMap | 0B000100000000;

  }
  else
  {
    // Send value
    tsdata.nullMap = tsdata.nullMap & 0B111011111111;

  }
  
  if (tsunit.fahrenheit)
  {
    tsdata.field4 = waterTempF;
    tsdata.field5 = weatherSensors.getAndResetTempF();
  }
  else
  {
    // Convert to C
    tsdata.field4 = (waterTempF-32.0)*5.0/9.0;
    tsdata.field5 = (weatherSensors.getAndResetTempF()-32.0)*5.0/9.0;
  }
  tsdata.field6 = (float)(int)weatherSensors.getAndResetHumidityRH();
  // Don't send RH if not correct
  if (tsdata.field6 < 1.0)
  {
    // Send null
    tsdata.nullMap = tsdata.nullMap | 0B000001000000;
  }
  else
  {
    // Send value
    tsdata.nullMap = tsdata.nullMap & 0B111110111111;
  }
  float pressurePascals = (float)weatherSensors.getAndResetPressurePascals();
  tsdata.field7 = pressurePascals*tsunit.pressurefactor;
  // Don't send pressure if not correct
  if (pressurePascals < 90000.0 || pressurePascals > 110000.0)
  {
    // Send null
    tsdata.nullMap = tsdata.nullMap | 0B000000100000;
  }
  else
  {
    // Send value
    tsdata.nullMap = tsdata.nullMap & 0B111111011111;
  }
  tsdata.field8 = (float)weatherSensors.getAndResetBatteryMV()/1000;
  // Don't send voltage if not correct
  if (tsdata.field8 < 0.1)
  {
    // Send null
    tsdata.nullMap = tsdata.nullMap | 0B000000010000;
  }
  else
  {
    // Send value
    tsdata.nullMap = tsdata.nullMap & 0B111111101111;
  }  
  String statusmessage = String::format("%.2f",tide.mhhw()*tsunit.levelfactor) + ":"
                        + String::format("%.2f",tide.msl()*tsunit.levelfactor) + ":"
                        + String::format("%.2f",tide.mllw()*tsunit.levelfactor) +":"
                        + String::format("%.1f",gustMPH*tsunit.speedfactor)
  ;
  strcpy(tsdata.status,statusmessage.c_str());
  readings =
    String(readingTime) + "," +
    String(tsdata.field1) + "," +
    String(tsdata.field2) + "," +
    String(tsdata.field3) + "," +
    String(tsdata.field4) + "," +
    String(tsdata.field5) + "," +
    String(tsdata.field6) + "," +
    String(tsdata.field7) + "," +
    String(tsdata.field8) + "," +
    String(tsdata.status);
  DEBUG_PRINTLN(readings);
  printToSd.println(readings);

  thingSpeak.queueToSend((u_int8_t*)&tsdata);
  node.tickleWatchdog();
  Particle.publishVitals();  // Publish vitals immmediately 
}

// Adding explicit connect routine that has to work before the rest of the code runs
void connect()
{
  #if Wiring_Cellular
  bool cellready=Cellular.ready();
  if (!cellready)
  {
    DEBUG_PRINTLN("Attempting to connect cellular...");
    Cellular.on();
    Cellular.connect();
    // Increased timeout for Boron 3G that seems to take a long time
    waitFor(Cellular.ready,600000);
    if (!Cellular.ready())
    {
    DEBUG_PRINTLN("Cellular not ready - resetting");
    delay(200);
    System.reset();
    }
  }
  else
  {
    DEBUG_PRINTLN("Cellular ready");
  }
  #endif
  
  #if Wiring_WiFi
  if (!WiFi.hasCredentials())
  {
    DEBUG_PRINTLN("Please add WiFi credentials");
    DEBUG_PRINTLN("Resetting in 60 seconds");
    delay(60000);
    System.reset();
  }
  bool wifiready=WiFi.ready();
  if (!wifiready)
  {
    DEBUG_PRINTLN("Attempting to connect to WiFi...");
    WiFi.on();
    WiFi.connect();
    waitFor(WiFi.ready,60000);
    if (!WiFi.ready())
    {
    DEBUG_PRINTLN("WiFi not ready - resetting");
    delay(200);
    System.reset();
    }
  }
  else
  {
    DEBUG_PRINTLN("WiFi ready");
  }
  #endif

  bool partconnected=Particle.connected();
  if (!partconnected)
  {
    DEBUG_PRINTLN("Attempting to connect to Particle...");
    Particle.connect();
    // Note: that conditions must be a function that takes a void argument function(void) with the () removed,
    // e.g. Particle.connected instead of Particle.connected().
    waitFor(Particle.connected,90000);
    if (!Particle.connected())
    {
      DEBUG_PRINTLN("Particle not connected - resetting");
      delay(200);
      System.reset();
    } 
  }
  else
  {
    DEBUG_PRINTLN("Particle connected");
  }
}

bool syncRTC()
{
    bool sync = false;
    unsigned long syncTimer = millis();

    do
    {
      Particle.process();
      delay(100);
    } while (Time.now() < 1465823822 && millis()-syncTimer<500);

    if (Time.now() > 1465823822)
    {
        syncTime = Time.now();//put time into memory
        node.setUnixTime(syncTime);
        sync = true;
    }

    if (!sync)
    {
        #ifdef DEBUG
        Particle.publish("Time NOT synced",String(Time.format(syncTime, TIME_FORMAT_ISO8601_FULL)+"  "+Time.format(node.unixTime(), TIME_FORMAT_ISO8601_FULL)),PRIVATE);
        DEBUG_PRINT("Time NOT synced: ");
        DEBUG_PRINTLN(String(Time.format(syncTime, TIME_FORMAT_ISO8601_FULL)+"  "+Time.format(node.unixTime(), TIME_FORMAT_ISO8601_FULL)));
        #endif
    }
    return sync;
}

String deviceStatus;
String i2cDevices = "";
bool resetDevice = false;

String i2cNames[] =
{
    "RTC",
    "Exp",
    "RTC EEPROM",
    "ADC",
    "FRAM",
    "AM2315",
    "MPL3115"
};

byte i2cAddr[]=
{
    0x6F, //111
    0x20, //32
    0x57, //87
    0x4D, //77
    0x50, //80
    0x5C, //
    0x60
    
};

/* number of elements in `array` */
static const size_t i2cLength = sizeof(i2cAddr) / sizeof(i2cAddr[0]);

bool i2cExists[]=
{
  false,
  false,
  false,
  false,
  false,
  false,
  false
};

// check i2c devices with i2c names at i2c address of length i2c length returned in i2cExists
bool checkI2CDevices()
{
  byte error, address;
  bool result = true;
  i2cDevices = "";
  for (size_t i=0; i<i2cLength; ++i)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    address = i2cAddr[i];
    WITH_LOCK(Wire)
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
    }
    //Try again if !error=0
    if (!error==0)
    {
      delay(10);
      WITH_LOCK(Wire)
      {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
      }
    }

    //Try reset if !error=0
    if (!error==0)
    {
      WITH_LOCK(Wire)
      {      
        Wire.reset();
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
      }
    }
 
    if (error == 0)
    {
      DEBUG_PRINTLN(String("Device "+i2cNames[i]+ " at"+" address:0x"+String(address, HEX)));
      i2cExists[i]=true;
      i2cDevices.concat(i2cNames[i]);
      i2cDevices.concat(":");
    }
    else
    {
      DEBUG_PRINTLN(String("Device "+i2cNames[i]+ " NOT at"+" address:0x"+String(address, HEX)));
      i2cExists[i]=false;
      result = false;
      i2cDevices.concat(i2cNames[i]);
      i2cDevices.concat("-missing:");
    }
  }
  return result;
}


void printI2C(int inx)
{
    for (uint8_t i=0; i<i2cLength; i++)
        {
          if (i2cAddr[i] == inx)
          {
              DEBUG_PRINTLN(String("Device "+i2cNames[i]+ " at"+" address:0x"+String(i2cAddr[i], HEX)));
          }
        }        
}

void scanI2C()
{
  byte error, address;
  int nDevices;
 
  DEBUG_PRINTLN("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    WITH_LOCK(Wire)
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
    }
 
    if (error == 0)
    {
      printI2C(address);
 
      nDevices++;
    }
    else if (error==4)
    {
      DEBUG_PRINT("Unknown error at address 0x");
      if (address<16)
        DEBUG_PRINT("0");
      DEBUG_PRINTLN(address,HEX);
    }    
  }
  if (nDevices == 0)
    DEBUG_PRINTLN("No I2C devices found\n");
  else
    DEBUG_PRINTLN("done\n");
}

uint32_t lastthingspeakmillis=0;
String startupStatus;
bool setupError = false;
bool startupMessSent = false;
uint32_t doneMillis;
String setupSec;
bool resetNow = false;
bool clearCalibration = false;
uint32_t resetDelay;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup() runs once, when the device is first turned on.
void setup() {
  Particle.function("systemreset",systemreset);
  Particle.function("hardreset", hardreset);
  Particle.function("setunits", setunits);
  Particle.variable("currentReadings",readings);
  Particle.variable("devices",deviceString);
  Particle.variable("sonarRanges",maxbotixranges);
  Particle.variable("mllwCalibration",mllwCalibrationMess);

  // Used for debug
  // pinMode(D5,OUTPUT);
  // digitalWrite(D5,HIGH);

  pinMode(D1, OUTPUT);
  for (int i = 0; i < 15; i++) {
    digitalWrite(D1, HIGH);
    delayMicroseconds(100);
    digitalWrite(D1, LOW);
    delayMicroseconds(100);
  }
  pinMode(D1, INPUT);

  Serial.begin(115200);
  #ifdef SERIAL_DEBUG
  delay(1000);
  #endif
  DEBUG_PRINTLN("Starting");

  Wire.begin();

  if (!node.begin())
  {
    DEBUG_PRINTLN("IoT Node expander not responding. Resetting after connection is made.");
    startupStatus = "IoT Node Error. ";
    setupError = true;
    // if (waitFor(Particle.connected, 10000)) 
    // {
    //   Particle.publish("Error","IoT Node expander not responding. Resetting after connection is made..",PRIVATE);
    // }
    // uint32_t millisNow = millis();
    // while (millis()-millisNow < 60000)
    // {
    //   blinkOrange.setActive(true);
    //   delay(500);
    //   blinkOrange.setActive(false);
    //   blinkYellow.setActive(true);
    //   delay(500);
    //   blinkYellow.setActive(false);
    //   blinkOrange.setActive(true);
    //   delay(500);
    // }
    // delay(1000);
    // System.reset();
  }
  else
  {
    DEBUG_PRINTLN("IoT Node expander responded");
    startupStatus = "IoT Node OK. ";
  }

  node.setPowerON(EXT3V3, true);

  node.setPowerON(EXT5V, true);

  delay(1000);

  checkI2CDevices();

  deviceString = i2cDevices;

  if (!node.ok())
  {
    DEBUG_PRINTLN("IoT Node not connected. Resetting after connection is made.");
    startupStatus.concat("IoT Node not connected. ");
    setupError = true;   
  }
  else
  {
    DEBUG_PRINTLN("IoT Node connected");
    startupStatus.concat("IoT Node connected. ");
  }

  // Check three times if needed for additional reliability
  int checktimes = 0;
  bool unitsokay = false;
  do
  {
    framtsunit.read(0,(uint8_t*)&tsunit);
    if (tsunit.firstrun==firstrunvalue)
    {
      unitsokay=true;
    }
    checktimes++;
  } while ((unitsokay==false)&&(checktimes<3));

  // DEBUG_PRINTLNF("%1.1f",tsunit.speedfactor);
  // DEBUG_PRINTLNF("%1.10f",tsunit.levelfactor);
  // DEBUG_PRINTLNF("%1.8f",tsunit.fahrenheit);
  // DEBUG_PRINTLNF("%1.10f",tsunit.pressurefactor);
  // DEBUG_PRINTLN(tsunit.firstrun);
  // DEBUG_PRINTLN(unitsokay);

  // Set units if the first time running
  if (!unitsokay)
  {
    tsunit.firstrun=firstrunvalue;
    DEBUG_PRINT("Level factor = ");
    DEBUG_PRINTLN(tsunit.levelfactor);
    // Then first run
    DEBUG_PRINT("Setting units to: ");
    // US Options
    #ifdef UNITS_US
      tsunit.speedfactor = 1; // mph
      tsunit.levelfactor = 0.00328084; // mm to feet
      tsunit.fahrenheit = true;
      tsunit.pressurefactor = 0.01; // millibars
      DEBUG_PRINTLN("US");
    #endif
    // Use a function setunits to change these to metric
    #ifdef UNITS_METRIC
    // Metric options
      tsunit.speedfactor = 0.44704; // m/s
      tsunit.levelfactor = 0.001; // mm to meters
      tsunit.fahrenheit = false;
      tsunit.pressurefactor = 0.01; // hectopascals
      DEBUG_PRINTLN("metric");
      #endif
    framtsunit.write(0,(uint8_t*)&tsunit); 
  }

  DEBUG_PRINTLN("Starting Weather Sensors");
  weatherSensors.begin();

  delay(50);
  weatherSensors.captureWaterTemp();
  deviceString.concat(weatherSensors.dsType());
  deviceString.concat(":");

  if (!(maxbotix.setup()>0))
  {
    DEBUG_PRINTLN("No Maxbotix sensor connected. Resetting after connection is made.");
    startupStatus.concat("No Maxbotix sensor. ");
    setupError = true;
  }
  else
  {
    DEBUG_PRINTLN("Maxbotix sensors connected");
    startupStatus.concat("Maxbotix connected. ");
  }
  
  if (maxbotix.dualSensor)
  {
    DEBUG_PRINT("Maxbotix sensor 1: MB");
    DEBUG_PRINTLN(maxbotix.sensor1ModelNum);
    deviceString.concat("MB");
    deviceString.concat(String(maxbotix.sensor1ModelNum));
    deviceString.concat(":");
    DEBUG_PRINT("Maxbotix sensor 2: MB");    
    DEBUG_PRINTLN(maxbotix.sensor2ModelNum);
    deviceString.concat("MB");
    deviceString.concat(String(maxbotix.sensor2ModelNum));
    deviceString.concat(":");
  }
  else
  {
    DEBUG_PRINT("Maxbotix sensor 1: MB");
    DEBUG_PRINTLN(maxbotix.sensor1ModelNum);
    deviceString.concat("MB");
    deviceString.concat(String(maxbotix.sensor1ModelNum));
    deviceString.concat(":");
  }

  thingSpeak.setup();
  bool tideinitialized = false;
  if (tide.initialize())
  {
    DEBUG_PRINTLN("Tide initialized");
    startupStatus.concat("Tide initialized. ");
    tideinitialized = true;
  }
  else
  {
    DEBUG_PRINTLN("Tide not initialized");
    startupStatus.concat("Tide not initialized. ");
  }

  String debugmessage;
  if (maxbotix.dualSensor)
  {
    if(!maxbotix.isDualSensorCalibrated())
    {      
      maxbotix.calibrateDualSensorOffSet();
      DEBUG_PRINTLN("Calibrating sonar.");
      debugmessage = "Maxbotix sensors offset: " + String(maxbotix.calib.dualSensorOffset) + " mm. ";
      DEBUG_PRINTLN(debugmessage);
      startupStatus.concat(debugmessage);
    }
    else
    {
      debugmessage = "Maxbotix sensors offset: " + String(maxbotix.calib.dualSensorOffset) + " mm. ";
      DEBUG_PRINTLN(debugmessage);
    }
  }

  startupStatus.concat(deviceString);

  if (!(node.unixTime() >= syncTime))
  {
    if (setupError)
    {
      DEBUG_PRINTLN("Time not set.");
      startupStatus.concat("Time not set. ");
    }
    else
    {
      // Connect and set time before continuing
      connect();
      if (syncRTC())
      {
        DEBUG_PRINTLN("Time reset.");
        startupStatus.concat("Time reset. ");
      }
      else
      {
        DEBUG_PRINTLN("Time not set.");
        startupStatus.concat("Time not set.");
        setupError = true;
      }
    }
  }

  if (setupError)
  {
    connect();
    syncRTC();
    doneMillis = millis();
    setupSec = String::format("Setup seconds: %d .",doneMillis/1000);
    DEBUG_PRINTLN(setupSec);
    startupStatus.concat(setupSec);
    startupStatus.concat("Startup Error. Resetting.");
    Particle.publish("Startup status",startupStatus,PRIVATE);
    DEBUG_PRINTLN(startupStatus);
    delay(3000);
    System.reset();
  }
  else
  {
    if (Particle.connected()) 
    {
      syncRTC();
      doneMillis = millis();
      setupSec = String::format("Setup seconds: %d .",doneMillis/1000);
      DEBUG_PRINTLN(setupSec);
      startupStatus.concat(setupSec);
      Particle.publish("Startup status",startupStatus,PRIVATE);
      startupMessSent = true;
    }
    else
    {
      DEBUG_PRINTLN("Starting measurements while attempting to connect.");
    }
    
  }

  tsdata.nullMap = 0B000000001110;
  readSensorTimer.start();
  sendSensorTimer.start();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  maxbotix.readMaxbotixCharacter();

  if (Particle.connected())
  {
    // Particle.process();
    lastthingspeakmillis = thingSpeak.process();
  }
  // If message not sent to ThingSpeak in last 6 minutes then reset
  if (millis()-lastthingspeakmillis> SENSOR_SEND_RATE_MA*6+1000 )
  {
    DEBUG_PRINTLN("Resetting. ThingSpeak message unsuccessful in past 6 attempts.");
    if (Particle.connected())
    {
      Particle.publish("Resetting","ThingSpeak message unsuccessful in past 6 attempts.",PRIVATE);
    }
    delay(50);
    System.reset();
  }

  if (!startupMessSent)
  {
    if (Particle.connected())
    {
      doneMillis = millis();
      setupSec = String::format("Setup seconds: %d .",doneMillis/1000);
      DEBUG_PRINTLN(setupSec);
      startupStatus.concat(setupSec);
      Particle.publish("Startup status",startupStatus,PRIVATE);
      startupMessSent = true;
      DEBUG_PRINTLN("Connected");
    }
  }

  // Received hardreset from console so clear calibration
  if (clearCalibration)
  {
    tide.clear();
    maxbotix.clearDualSensorCalibration();
    Serial.println("Hard resetting!");
    if (Particle.connected())
    {
      Particle.publish("Hard Resetting","Hard reset message received.",PRIVATE);
    }
    resetDelay = millis();
    resetNow = true;
    clearCalibration = false;
    // delay(1000);
    // System.reset();
  }
  // Check to see hardreset is needed and enough time has passed since clearing calibration
  if (resetNow)
  {
    if(millis()-resetDelay>1000)
    {
      System.reset();
    }
  }

}

int hardreset(String resetcommand)
{
  if (resetcommand.equals("hardreset"))
  {
    clearCalibration = true;
  }
  return 0;
}

int systemreset(String command)
{
  if (command.equals("systemreset"))
  {
    System.reset();
  }
  return 0;  
}

int setunits(String type)
{
 tsunit.firstrun=firstrunvalue;
  if (type == "metric")
  {
    tsunit.speedfactor = 0.44704; // m/s
    tsunit.levelfactor = 0.001; // mm to meters
    tsunit.fahrenheit = false;
    tsunit.pressurefactor = 0.01; // hectopascals
    framtsunit.write(0,(uint8_t*)&tsunit);
    DEBUG_PRINTLN("Setting units to metric.");
  }
  if (type == "us")
  {
    tsunit.speedfactor = 1; // mph
    tsunit.levelfactor = 0.00328084; // mm to feet
    tsunit.fahrenheit = true;
    tsunit.pressurefactor = 0.01; // millibars
    framtsunit.write(0,(uint8_t*)&tsunit);
    DEBUG_PRINTLN("Setting units to us.");
  }
  return 0;
}