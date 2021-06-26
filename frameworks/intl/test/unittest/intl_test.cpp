/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "intl_test.h"
#include <gtest/gtest.h>
#include "date_time_format.h"
#include "locale_info.h"
#include "number_format.h"
#include <map>
#include <vector>

using namespace OHOS::Global::I18n;
using testing::ext::TestSize;
using namespace std;

namespace {
class IntlTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IntlTest::SetUpTestCase(void)
{}

void IntlTest::TearDownTestCase(void)
{}

void IntlTest::SetUp(void)
{}

void IntlTest::TearDown(void)
{}

/**
 * @tc.name: IntlFuncTest001
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest,  IntlFuncTest001, TestSize.Level1)
{
    string locale = "zh-CN-u-hc-h12";
    string expects = "公元2021年4月14日星期三 北美太平洋夏令时间 上午12:05:03";
    vector<string> locales;
    locales.push_back("jessie");
    locales.push_back(locale);
    map<string, string> options = { {"year", "numeric"},
                                {"month", "long"},
                                {"day", "numeric"},
                                {"hour", "numeric"},
                                {"minute", "2-digit"},
                                {"second", "numeric"},
                                {"weekday", "long"},
                                {"era", "short"},
                                {"timeZone", "America/Los_Angeles"},
                                {"timeZoneName", "long"}};
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = dateFormat->Format(2021, 3, 14, 15, 5, 3);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(dateFormat->GetYear(), "numeric");
    EXPECT_EQ(dateFormat->GetMonth(), "long");
    EXPECT_EQ(dateFormat->GetDay(), "numeric");
    EXPECT_EQ(dateFormat->GetHour(), "numeric");
    EXPECT_EQ(dateFormat->GetMinute(), "2-digit");
    EXPECT_EQ(dateFormat->GetSecond(), "numeric");
    EXPECT_EQ(dateFormat->GetWeekday(), "long");
    EXPECT_EQ(dateFormat->GetEra(), "short");
    EXPECT_EQ(dateFormat->GetHourCycle(), "h12");
    map<string, string> resolvedOptions = {};
    dateFormat->GetResolvedOptions(resolvedOptions);
    string str="{";
    typename map<string,string>::iterator it = resolvedOptions.begin();
    for (;it != resolvedOptions.end();it++) {
        str += "{";
        str += it->first + ": " + it->second;
        str += "}";
    }
    str += "}";
    string optionsString = "{{calendar: gregorian}{day: numeric}{era: short}{hour: numeric}{hourCycle: h12}";
    optionsString += "{locale: zh-CN}{minute: 2-digit}{month: long}{numberingSystem: latn}{second: numeric}";
    optionsString += "{timeZone: America/Los_Angeles}{timeZoneName: long}{weekday: long}{year: numeric}}";
    EXPECT_EQ(str, optionsString);
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest002
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest,  IntlFuncTest002, TestSize.Level1)
{
    string locale = "ja-Jpan-JP-u-ca-japanese-hc-h12-co-emoji";
    map<string, string> options = { {"hourCycle", "h11"},
                                {"numeric", "true"},
                                {"numberingSystem", "jpan"}};
    LocaleInfo *loc = new (std::nothrow) LocaleInfo(locale, options);
    if (loc == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    EXPECT_EQ(loc->GetLanguage(), "ja");
    EXPECT_EQ(loc->GetScript(), "Jpan");
    EXPECT_EQ(loc->GetRegion(), "JP");
    EXPECT_EQ(loc->GetBaseName(), "ja-Jpan-JP");
    EXPECT_EQ(loc->GetCalendar(), "japanese");
    EXPECT_EQ(loc->GetHourCycle(), "h11");
    EXPECT_EQ(loc->GetNumberingSystem(), "jpan");
    EXPECT_EQ(loc->Minimize()->GetScript(), "");
    EXPECT_EQ(loc->Minimize()->Maximize()->GetScript(), "Jpan");
    EXPECT_EQ(loc->GetNumeric(), "true");
    EXPECT_EQ(loc->GetCaseFirst(), "");
    delete loc;
}

/**
 * @tc.name: IntlFuncTest003
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest,  IntlFuncTest003, TestSize.Level1)
{
    string locale = "en-GB";
    LocaleInfo *loc = new (std::nothrow) LocaleInfo(locale);
    if (loc == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string language = "en";
    string script = "";
    string region = "GB";
    string baseName = "en-GB";
    EXPECT_EQ(loc->GetLanguage(), language);
    EXPECT_EQ(loc->GetScript(), script);
    EXPECT_EQ(loc->GetRegion(), region);
    EXPECT_EQ(loc->GetBaseName(), baseName);
    delete loc;
}

/**
 * @tc.name: IntlFuncTest001
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest,  IntlFuncTest004, TestSize.Level1)
{
    string locale = "en-GB";
    string expects = "14 April 2021 at 15:05";
    vector<string> locales;
    locales.push_back(locale);
    string dateStyle = "long";
    string timeStyle = "short";
    map<string, string> options = { {"dateStyle", dateStyle},
                                {"timeStyle", timeStyle}};
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = dateFormat->Format(2021, 3, 14, 15, 5, 3);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(dateFormat->GetDateStyle(), dateStyle);
    EXPECT_EQ(dateFormat->GetTimeStyle(), timeStyle);
    map<string, string> resolvedOptions = {};
    dateFormat->GetResolvedOptions(resolvedOptions);
    string str="{";
    typename map<string,string>::iterator it = resolvedOptions.begin();
    for (;it != resolvedOptions.end();it++) {
        str += "{";
        str += it->first + ": " + it->second;
        str += "}";
    }
    str += "}";
    std::string optionsString = "{{calendar: gregorian}{dateStyle: long}{locale: en-GB}{numberingSystem: latn}";
    optionsString += "{timeStyle: short}{timeZone: Asia/Shanghai}}";
    EXPECT_EQ(str, optionsString);
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest,  IntlFuncTest005, TestSize.Level1)
{
    string locale = "en-IN";
    string expects = "€01,23,456.79";
    vector<string> locales;
    locales.push_back(locale);
    string useGrouping = "true";
    string minimumIntegerDigits = "7";
    string maximumFractionDigits = "2";
    string style = "currency";
    string currency = "EUR";
    map<string, string> options = { {"useGrouping", useGrouping},
                                {"minimumIntegerDigits", minimumIntegerDigits},
                                {"maximumFractionDigits", maximumFractionDigits},
                                {"style", style},
                                {"currency", currency}};
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetUseGrouping(), useGrouping);
    EXPECT_EQ(numFmt->GetMinimumIntegerDigits(), minimumIntegerDigits);
    EXPECT_EQ(numFmt->GetMaximumFractionDigits(), maximumFractionDigits);
    EXPECT_EQ(numFmt->GetStyle(), style);
    EXPECT_EQ(numFmt->GetCurrency(), currency);
    delete numFmt;
}
}
