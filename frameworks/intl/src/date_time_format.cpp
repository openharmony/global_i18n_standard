/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "date_time_format.h"
#include <algorithm>
#include <cmath>
#include "ohos/init_data.h"
#include "locale_config.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace icu;

bool DateTimeFormat::icuInitialized = DateTimeFormat::Init();

std::map<std::string, DateFormat::EStyle> DateTimeFormat::dateTimeStyle = {
    { "full", DateFormat::EStyle::kFull },
    { "long", DateFormat::EStyle::kLong },
    { "medium", DateFormat::EStyle::kMedium },
    { "short", DateFormat::EStyle::kShort }
};

DateTimeFormat::DateTimeFormat(const std::vector<std::string> &localeTags, std::map<std::string, std::string> &configs)
{
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::LocaleBuilder> builder = nullptr;
    builder = std::make_unique<LocaleBuilder>();
    ParseConfigsPartOne(configs);
    ParseConfigsPartTwo(configs);
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = Locale::forLanguageTag(StringPiece(curLocale), status);
        if (LocaleInfo::allValidLocales.count(locale.getLanguage()) > 0) {
            InitWithLocale(curLocale, configs);
            if (!dateFormat) {
                delete localeInfo;
                localeInfo = nullptr;
                continue;
            }
            break;
        }
    }
    if (!localeInfo || !dateFormat) {
        InitWithDefaultLocale(configs);
    }
}

DateTimeFormat::~DateTimeFormat()
{
    if (dateIntvFormat != nullptr) {
        delete dateIntvFormat;
        dateIntvFormat = nullptr;
    }
    if (calendar != nullptr) {
        delete calendar;
        calendar = nullptr;
    }
    if (dateFormat != nullptr) {
        delete dateFormat;
        dateFormat = nullptr;
    }
    if (localeInfo != nullptr) {
        delete localeInfo;
        localeInfo = nullptr;
    }
}

bool DateTimeFormat::CheckInitSuccess()
{
    if (dateIntvFormat == nullptr || calendar == nullptr || dateFormat == nullptr || localeInfo == nullptr) {
        return false;
    }
    return true;
}

std::unique_ptr<DateTimeFormat> DateTimeFormat::CreateInstance(const std::vector<std::string> &localeTags,
                                                               std::map<std::string, std::string> &configs)
{
    std::unique_ptr<DateTimeFormat> dateTimeFormat = std::make_unique<DateTimeFormat>(localeTags, configs);
    if (!dateTimeFormat->CheckInitSuccess()) {
        return nullptr;
    }
    return dateTimeFormat;
}

void DateTimeFormat::InitWithLocale(const std::string &curLocale, std::map<std::string, std::string> &configs)
{
    UErrorCode status = U_ZERO_ERROR;
    localeInfo = new LocaleInfo(curLocale, configs);
    locale = localeInfo->GetLocale();
    localeTag = localeInfo->GetBaseName();
    if (hourCycle.empty()) {
        hourCycle = localeInfo->GetHourCycle();
    }
    if (hour12.empty() && hourCycle.empty()) {
        bool is24HourClock = LocaleConfig::Is24HourClock();
        if (is24HourClock) {
            hour12 = "false";
        }
    }
    ComputeHourCycleChars();
    ComputeSkeleton();
    if (!configs.size()) {
        InitDateFormatWithoutConfigs(status);
    } else {
        InitDateFormat(status);
    }
    calendar = Calendar::createInstance(locale, status);
}

void DateTimeFormat::InitWithDefaultLocale(std::map<std::string, std::string> &configs)
{
    if (localeInfo != nullptr) {
        delete localeInfo;
        localeInfo = nullptr;
    }
    if (dateFormat != nullptr) {
        delete dateFormat;
        dateFormat = nullptr;
    }
    InitWithLocale(LocaleConfig::GetSystemLocale(), configs);
}

void DateTimeFormat::InitDateFormatWithoutConfigs(UErrorCode &status)
{
    dateFormat = DateFormat::createDateInstance(DateFormat::SHORT, locale);
    SimpleDateFormat *simDateFormat = static_cast<SimpleDateFormat*>(dateFormat);
    if (simDateFormat != nullptr) {
        simDateFormat->toPattern(pattern);
    }
    dateIntvFormat = DateIntervalFormat::createInstance(pattern, locale, status);
}

void DateTimeFormat::FixPatternPartOne()
{
    if (hour12 == "true") {
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("H")),
            icu::UnicodeString::fromUTF8(StringPiece("h")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("k")),
            icu::UnicodeString::fromUTF8(StringPiece("h")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("K")),
            icu::UnicodeString::fromUTF8(StringPiece("h")));
    } else if (hour12 == "false") {
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("h")),
            icu::UnicodeString::fromUTF8(StringPiece("H")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("k")),
            icu::UnicodeString::fromUTF8(StringPiece("H")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("K")),
            icu::UnicodeString::fromUTF8(StringPiece("H")));
        removeAmPmChar();
    } else if (hourCycle != "") {
        FixPatternPartTwo();
    }
}

void DateTimeFormat::FixPatternPartTwo()
{
    if (hourCycle == "h11") {
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("h")),
            icu::UnicodeString::fromUTF8(StringPiece("k")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("H")),
            icu::UnicodeString::fromUTF8(StringPiece("k")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("K")),
            icu::UnicodeString::fromUTF8(StringPiece("k")));
    } else if (hourCycle == "h12") {
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("H")),
            icu::UnicodeString::fromUTF8(StringPiece("h")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("k")),
            icu::UnicodeString::fromUTF8(StringPiece("h")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("K")),
            icu::UnicodeString::fromUTF8(StringPiece("h")));
    } else if (hourCycle == "h23") {
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("h")),
            icu::UnicodeString::fromUTF8(StringPiece("K")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("H")),
            icu::UnicodeString::fromUTF8(StringPiece("K")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("k")),
            icu::UnicodeString::fromUTF8(StringPiece("K")));
        removeAmPmChar();
    } else if (hourCycle == "h24") {
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("h")),
            icu::UnicodeString::fromUTF8(StringPiece("H")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("k")),
            icu::UnicodeString::fromUTF8(StringPiece("H")));
        pattern.findAndReplace(icu::UnicodeString::fromUTF8(StringPiece("K")),
            icu::UnicodeString::fromUTF8(StringPiece("H")));
        removeAmPmChar();
    }
}

void DateTimeFormat::removeAmPmChar()
{
    std::string patternString = "";
    pattern.toUTF8String(patternString);
    size_t amPmCharStartIdx = 0;
    size_t amPmCharEndIdx = 0;
    for (size_t i = 0; i < patternString.length(); i++) {
        if (patternString[i] != 'a') {
            continue;
        }
        if ((i + 1) < patternString.length() && patternString[i + 1] == 't') {
            continue;
        }
        if (!i) {
            amPmCharStartIdx = i;
        } else {
            amPmCharStartIdx = i - 1;
            while (amPmCharStartIdx > 0 && patternString[amPmCharStartIdx] == ' ') {
                amPmCharStartIdx -= 1;
            }
            if (amPmCharStartIdx || patternString[amPmCharStartIdx] != ' ') {
                amPmCharStartIdx += 1;
            }
        }
        amPmCharEndIdx = i + 1;
        while (amPmCharEndIdx < patternString.length() && patternString[amPmCharEndIdx] == ' ') {
            amPmCharEndIdx += 1;
        }
        break;
    }
    size_t length = amPmCharEndIdx - amPmCharStartIdx;
    if (length) {
        if (!amPmCharStartIdx || amPmCharEndIdx == patternString.length()) {
            patternString = patternString.replace(amPmCharStartIdx, length, "");
        } else {
            patternString = patternString.replace(amPmCharStartIdx, length, " ");
        }
        pattern = icu::UnicodeString(patternString.data(), patternString.length());
    }
}

void DateTimeFormat::InitDateFormat(UErrorCode &status)
{
    if (!dateStyle.empty() || !timeStyle.empty()) {
        DateFormat::EStyle dateStyleValue = DateFormat::EStyle::kNone;
        DateFormat::EStyle timeStyleValue = DateFormat::EStyle::kNone;
        if (!dateStyle.empty() && dateTimeStyle.count(dateStyle) > 0) {
            dateStyleValue = dateTimeStyle[dateStyle];
        }
        if (!timeStyle.empty() && dateTimeStyle.count(timeStyle) > 0) {
            timeStyleValue = dateTimeStyle[timeStyle];
        }
        dateFormat = DateFormat::createDateTimeInstance(dateStyleValue, timeStyleValue, locale);
        SimpleDateFormat *simDateFormat = static_cast<SimpleDateFormat*>(dateFormat);
        if (simDateFormat != nullptr) {
            simDateFormat->toPattern(pattern);
        }
        FixPatternPartOne();
        delete dateFormat;
        dateFormat = new SimpleDateFormat(pattern, locale, status);
    } else {
        auto patternGenerator =
            std::unique_ptr<DateTimePatternGenerator>(DateTimePatternGenerator::createInstance(locale, status));
        ComputePattern();
        pattern =
            patternGenerator->replaceFieldTypes(patternGenerator->getBestPattern(pattern, status), pattern, status);
        pattern = patternGenerator->getBestPattern(pattern, status);
        dateFormat = new SimpleDateFormat(pattern, locale, status);
    }
    dateIntvFormat = DateIntervalFormat::createInstance(pattern, locale, status);
}

void DateTimeFormat::ParseConfigsPartOne(std::map<std::string, std::string> &configs)
{
    if (configs.count("dateStyle") > 0) {
        dateStyle = configs["dateStyle"];
    }
    if (configs.count("timeStyle") > 0) {
        timeStyle = configs["timeStyle"];
    }
    if (configs.count("year") > 0) {
        year = configs["year"];
    }
    if (configs.count("month") > 0) {
        month = configs["month"];
    }
    if (configs.count("day") > 0) {
        day = configs["day"];
    }
    if (configs.count("hour") > 0) {
        hour = configs["hour"];
    }
    if (configs.count("minute") > 0) {
        minute = configs["minute"];
    }
    if (configs.count("second") > 0) {
        second = configs["second"];
    }
}

void DateTimeFormat::ParseConfigsPartTwo(std::map<std::string, std::string> &configs)
{
    if (configs.count("hourCycle") > 0) {
        hourCycle = configs["hourCycle"];
    }
    if (configs.count("timeZone") > 0) {
        timeZone = configs["timeZone"];
    }
    if (configs.count("numberingSystem") > 0) {
        numberingSystem = configs["numberingSystem"];
    }
    if (configs.count("hour12") > 0) {
        hour12 = configs["hour12"];
    }
    if (configs.count("weekday") > 0) {
        weekday = configs["weekday"];
    }
    if (configs.count("era") > 0) {
        era = configs["era"];
    }
    if (configs.count("timeZoneName") > 0) {
        timeZoneName = configs["timeZoneName"];
    }
    if (configs.count("dayPeriod") > 0) {
        dayPeriod = configs["dayPeriod"];
    }
    if (configs.count("localeMatcher") > 0) {
        localeMatcher = configs["localeMatcher"];
    }
    if (configs.count("formatMatcher") > 0) {
        formatMatcher = configs["formatMatcher"];
    }
}

void DateTimeFormat::ComputeSkeleton()
{
    if (year.empty() && month.empty() && day.empty() && hour.empty() && minute.empty() && second.empty()) {
        pattern.append("yMd");
    }
    AddOptions(year, yearChar);
    AddOptions(month, monthChar);
    AddOptions(day, dayChar);
    AddOptions(hour, hourChar);
    AddOptions(minute, minuteChar);
    AddOptions(second, secondChar);
    if ((hourCycle == "h12" || hourCycle == "h11" || hour12 == "true") && !hour.empty()) {
        pattern.append(amPmChar);
    }
    AddOptions(timeZoneName, timeZoneChar);
    AddOptions(weekday, weekdayChar);
    AddOptions(era, eraChar);
}

void DateTimeFormat::AddOptions(std::string option, char16_t optionChar)
{
    if (!option.empty()) {
        pattern.append(optionChar);
    }
}

void DateTimeFormat::ComputeHourCycleChars()
{
    if (!hour12.empty()) {
        if (hour12 == "true") {
            hourNumericString = "h";
            hourTwoDigitString = "hh";
        } else {
            hourNumericString = "H";
            hourTwoDigitString = "HH";
        }
    } else {
        if (hourCycle == "h11") {
            hourNumericString = "K";
            hourTwoDigitString = "KK";
        } else if (hourCycle == "h12") {
            hourNumericString = "h";
            hourTwoDigitString = "hh";
        } else if (hourCycle == "h23") {
            hourNumericString = "H";
            hourTwoDigitString = "HH";
        } else if (hourCycle == "h24") {
            hourNumericString = "k";
            hourTwoDigitString = "kk";
        }
    }
}

void DateTimeFormat::ComputePattern()
{
    ComputePartOfPattern(year, yearChar, "yy", "Y");
    ComputePartOfPattern(day, dayChar, "dd", "d");
    ComputePartOfPattern(hour, hourChar, hourTwoDigitString, hourNumericString);
    ComputePartOfPattern(minute, minuteChar, "mm", "mm");
    ComputePartOfPattern(second, secondChar, "ss", "ss");
    if (!month.empty()) {
        UnicodeString monthOfPattern = UnicodeString(monthChar);
        int32_t length = monthOfPattern.length();
        if (month == "numeric" && length != NUMERIC_LENGTH) {
            pattern.findAndReplace(monthOfPattern, UnicodeString::fromUTF8(StringPiece("M")));
        } else if (month == "2-digit" && length != TWO_DIGIT_LENGTH) {
            pattern.findAndReplace(monthOfPattern, UnicodeString::fromUTF8(StringPiece("MM")));
        } else if (month == "long" && length != LONG_LENGTH) {
            pattern.findAndReplace(monthOfPattern, UnicodeString::fromUTF8(StringPiece("MMMM")));
        } else if (month == "short" && length != SHORT_LENGTH) {
            pattern.findAndReplace(monthOfPattern, UnicodeString::fromUTF8(StringPiece("MMM")));
        } else if (month == "narrow" && length != NARROW_LENGTH) {
            pattern.findAndReplace(monthOfPattern, UnicodeString::fromUTF8(StringPiece("MMMMM")));
        }
    }
    if (!timeZoneName.empty()) {
        UnicodeString timeZoneOfPattern = UnicodeString(timeZoneChar);
        if (timeZoneName == "long") {
            pattern.findAndReplace(timeZoneOfPattern, UnicodeString::fromUTF8(StringPiece("zzzz")));
        } else if (timeZoneName == "short") {
            pattern.findAndReplace(timeZoneOfPattern, UnicodeString::fromUTF8(StringPiece("O")));
        }
    }

    ComputeWeekdayOrEraOfPattern(weekday, weekdayChar, "EEEE", "E", "EEEEE");
    ComputeWeekdayOrEraOfPattern(era, eraChar, "GGGG", "G", "GGGGG");
}

void DateTimeFormat::ComputePartOfPattern(std::string option, char16_t character, std::string twoDigitChar,
    std::string numericChar)
{
    if (!option.empty()) {
        UnicodeString curPartOfPattern = UnicodeString(character);
        int32_t length = curPartOfPattern.length();
        if (option == "2-digit" && length == TWO_DIGIT_LENGTH) {
            pattern.findAndReplace(curPartOfPattern, UnicodeString::fromUTF8(StringPiece(twoDigitChar)));
        } else if (option == "numeric" && length == NUMERIC_LENGTH) {
            pattern.findAndReplace(curPartOfPattern, UnicodeString::fromUTF8(StringPiece(numericChar)));
        }
    }
}

void DateTimeFormat::ComputeWeekdayOrEraOfPattern(std::string option, char16_t character, std::string longChar,
    std::string shortChar, std::string narrowChar)
{
    if (!option.empty()) {
        UnicodeString curPartOfPattern = UnicodeString(character);
        int32_t length = curPartOfPattern.length();
        if (option == "long" && length != LONG_ERA_LENGTH) {
            pattern.findAndReplace(curPartOfPattern, UnicodeString::fromUTF8(StringPiece(longChar)));
        } else if (option == "short" && length != SHORT_ERA_LENGTH) {
            pattern.findAndReplace(curPartOfPattern, UnicodeString::fromUTF8(StringPiece(shortChar)));
        } else if (option == "narrow" && length != NARROW_LENGTH) {
            pattern.findAndReplace(curPartOfPattern, UnicodeString::fromUTF8(StringPiece(narrowChar)));
        }
    }
}

int64_t DateTimeFormat::GetArrayValue(int64_t *dateArray, size_t index, size_t size)
{
    if (index < size) {
        return dateArray[index];
    } else {
        return 0;
    }
}

std::string DateTimeFormat::Format(int64_t *date, size_t size)
{
    UErrorCode status = U_ZERO_ERROR;
    std::string result;
    UnicodeString dateString;
    int64_t year = GetArrayValue(date, YEAR_INDEX, size);
    int64_t month = GetArrayValue(date, MONTH_INDEX, size);
    int64_t day = GetArrayValue(date, DAY_INDEX, size);
    int64_t hour = GetArrayValue(date, HOUR_INDEX, size);
    int64_t minute = GetArrayValue(date, MINUTE_INDEX, size);
    int64_t second = GetArrayValue(date, SECOND_INDEX, size);
    calendar->clear();
    calendar->set(year, month, day, hour, minute, second);
    if (!timeZone.empty()) {
        UDate timestamp = calendar->getTime(status);
        auto zone = std::unique_ptr<TimeZone>(TimeZone::createTimeZone(timeZone.c_str()));
        calendar->setTimeZone(*zone);
        dateFormat->setTimeZone(*zone);
        calendar->setTime(timestamp, status);
    }
    dateFormat->format(calendar->getTime(status), dateString, status);
    dateString.toUTF8String(result);
    return result;
}

std::string DateTimeFormat::FormatRange(int64_t *fromDate, size_t fromDateSize, int64_t *toDate, size_t toDateSize)
{
    UErrorCode status = U_ZERO_ERROR;
    std::string result;
    UnicodeString dateString;
    int64_t year = GetArrayValue(fromDate, YEAR_INDEX, fromDateSize);
    int64_t month = GetArrayValue(fromDate, MONTH_INDEX, fromDateSize);
    int64_t day = GetArrayValue(fromDate, DAY_INDEX, fromDateSize);
    int64_t hour = GetArrayValue(fromDate, HOUR_INDEX, fromDateSize);
    int64_t minute = GetArrayValue(fromDate, MINUTE_INDEX, fromDateSize);
    int64_t second = GetArrayValue(fromDate, SECOND_INDEX, fromDateSize);
    calendar->clear();
    calendar->set(year, month, day, hour, minute, second);
    if (!timeZone.empty()) {
        UDate timestamp = calendar->getTime(status);
        auto zone = std::unique_ptr<TimeZone>(TimeZone::createTimeZone(timeZone.c_str()));
        calendar->setTimeZone(*zone);
        dateIntvFormat->setTimeZone(*zone);
        calendar->setTime(timestamp, status);
    }
    year = GetArrayValue(toDate, YEAR_INDEX, toDateSize);
    month = GetArrayValue(toDate, MONTH_INDEX, toDateSize);
    day = GetArrayValue(toDate, DAY_INDEX, toDateSize);
    hour = GetArrayValue(toDate, HOUR_INDEX, toDateSize);
    minute = GetArrayValue(toDate, MINUTE_INDEX, toDateSize);
    second = GetArrayValue(toDate, SECOND_INDEX, toDateSize);
    auto toCalendar = std::unique_ptr<Calendar>(Calendar::createInstance(locale, status));
    if (toCalendar == nullptr) {
        return nullptr;
    }
    toCalendar->clear();
    toCalendar->set(year, month, day, hour, minute, second);
    if (!timeZone.empty()) {
        UDate timestamp = toCalendar->getTime(status);
        auto zone = std::unique_ptr<TimeZone>(TimeZone::createTimeZone(timeZone.c_str()));
        toCalendar->setTimeZone(*zone);
        dateIntvFormat->setTimeZone(*zone);
        toCalendar->setTime(timestamp, status);
    }
    FieldPosition pos = 0;
    dateIntvFormat->format(*calendar, *toCalendar, dateString, pos, status);
    dateString.toUTF8String(result);
    return result;
}

void DateTimeFormat::GetResolvedOptions(std::map<std::string, std::string> &map)
{
    UErrorCode status = U_ZERO_ERROR;
    map.insert(std::make_pair("locale", localeTag));
    if (!(localeInfo->GetCalendar()).empty()) {
        map.insert(std::make_pair("calendar", localeInfo->GetCalendar()));
    } else {
        map.insert(std::make_pair("calendar", calendar->getType()));
    }
    if (!dateStyle.empty()) {
        map.insert(std::make_pair("dateStyle", dateStyle));
    }
    if (!timeStyle.empty()) {
        map.insert(std::make_pair("timeStyle", timeStyle));
    }
    if (!hourCycle.empty()) {
        map.insert(std::make_pair("hourCycle", hourCycle));
    } else if (!(localeInfo->GetHourCycle()).empty()) {
        map.insert(std::make_pair("hourCycle", localeInfo->GetHourCycle()));
    }
    if (!timeZone.empty()) {
        map.insert(std::make_pair("timeZone", timeZone));
    } else {
        UnicodeString timeZoneID("");
        std::string timeZoneString;
        dateFormat->getTimeZone().getID(timeZoneID).toUTF8String(timeZoneString);
        map.insert(std::make_pair("timeZone", timeZoneString));
    }
    if (!timeZoneName.empty()) {
        map.insert(std::make_pair("timeZoneName", timeZoneName));
    }
    if (!numberingSystem.empty()) {
        map.insert(std::make_pair("numberingSystem", numberingSystem));
    } else if (!(localeInfo->GetNumberingSystem()).empty()) {
        map.insert(std::make_pair("numberingSystem", localeInfo->GetNumberingSystem()));
    } else {
        auto numSys = std::unique_ptr<NumberingSystem>(NumberingSystem::createInstance(locale, status));
        map.insert(std::make_pair("numberingSystem", numSys->getName()));
    }
    GetAdditionalResolvedOptions(map);
}

void DateTimeFormat::GetAdditionalResolvedOptions(std::map<std::string, std::string> &map)
{
    if (!hour12.empty()) {
        map.insert(std::make_pair("hour12", hour12));
    }
    if (!weekday.empty()) {
        map.insert(std::make_pair("weekday", weekday));
    }
    if (!era.empty()) {
        map.insert(std::make_pair("era", era));
    }
    if (!year.empty()) {
        map.insert(std::make_pair("year", year));
    }
    if (!month.empty()) {
        map.insert(std::make_pair("month", month));
    }
    if (!day.empty()) {
        map.insert(std::make_pair("day", day));
    }
    if (!hour.empty()) {
        map.insert(std::make_pair("hour", hour));
    }
    if (!minute.empty()) {
        map.insert(std::make_pair("minute", minute));
    }
    if (!second.empty()) {
        map.insert(std::make_pair("second", second));
    }
    if (!dayPeriod.empty()) {
        map.insert(std::make_pair("dayPeriod", dayPeriod));
    }
    if (!localeMatcher.empty()) {
        map.insert(std::make_pair("localeMatcher", localeMatcher));
    }
    if (!formatMatcher.empty()) {
        map.insert(std::make_pair("formatMatcher", formatMatcher));
    }
}

std::string DateTimeFormat::GetDateStyle() const
{
    return dateStyle;
}

std::string DateTimeFormat::GetTimeStyle() const
{
    return timeStyle;
}

std::string DateTimeFormat::GetHourCycle() const
{
    return hourCycle;
}

std::string DateTimeFormat::GetTimeZone() const
{
    return timeZone;
}

std::string DateTimeFormat::GetTimeZoneName() const
{
    return timeZoneName;
}

std::string DateTimeFormat::GetNumberingSystem() const
{
    return numberingSystem;
}

std::string DateTimeFormat::GetHour12() const
{
    return hour12;
}

std::string DateTimeFormat::GetWeekday() const
{
    return weekday;
}

std::string DateTimeFormat::GetEra() const
{
    return era;
}

std::string DateTimeFormat::GetYear() const
{
    return year;
}

std::string DateTimeFormat::GetMonth() const
{
    return month;
}

std::string DateTimeFormat::GetDay() const
{
    return day;
}

std::string DateTimeFormat::GetHour() const
{
    return hour;
}

std::string DateTimeFormat::GetMinute() const
{
    return minute;
}

std::string DateTimeFormat::GetSecond() const
{
    return second;
}

bool DateTimeFormat::Init()
{
    SetHwIcuDirectory();
    return true;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
