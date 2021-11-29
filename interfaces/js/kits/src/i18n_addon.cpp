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
#include <vector>
#include <unordered_map>
#include "hilog/log.h"
#include "i18n_addon.h"
#include "i18n_calendar.h"
#include "node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "I18nJs" };
static napi_ref* g_constructor = nullptr;
static napi_ref* g_BrkConstructor = nullptr;
static std::unordered_map<std::string, UCalendarDateFields> g_fieldsMap {{
    { "era", UCAL_ERA },
    { "year", UCAL_YEAR },
    { "month", UCAL_MONTH },
    { "week_of_year", UCAL_WEEK_OF_YEAR },
    { "week_of_month", UCAL_WEEK_OF_MONTH },
    { "date", UCAL_DATE },
    { "day_of_year", UCAL_DAY_OF_YEAR },
    { "day_of_week", UCAL_DAY_OF_WEEK },
    { "day_of_week_in_month", UCAL_DAY_OF_WEEK_IN_MONTH },
    { "ap_pm", UCAL_AM_PM },
    { "hour", UCAL_HOUR },
    { "hour_of_day", UCAL_HOUR_OF_DAY },
    { "minute", UCAL_MINUTE },
    { "second", UCAL_SECOND },
    { "millisecond", UCAL_MILLISECOND },
    { "zone_offset", UCAL_ZONE_OFFSET },
    { "dst_offset", UCAL_DST_OFFSET },
    { "year_woy", UCAL_YEAR_WOY },
    { "dow_local", UCAL_DOW_LOCAL },
    { "extended_year", UCAL_EXTENDED_YEAR },
    { "julian_day", UCAL_JULIAN_DAY },
    { "milliseconds_in_day", UCAL_MILLISECONDS_IN_DAY },
    { "is_leap_month", UCAL_IS_LEAP_MONTH }}, 13
};
static std::unordered_map<std::string, CalendarType> g_typeMap {
    { "buddhist", CalendarType::BUDDHIST },
    { "chinese", CalendarType::CHINESE },
    { "coptic", CalendarType::COPTIC },
    { "ethiopic", CalendarType::ETHIOPIC },
    { "hebrew", CalendarType::HEBREW },
    { "gregory", CalendarType::GREGORY },
    { "indian", CalendarType::INDIAN },
    { "islamic_civil", CalendarType::ISLAMIC_CIVIL },
    { "islamic_tbla", CalendarType::ISLAMIC_TBLA },
    { "islamic_umalqura", CalendarType::ISLAMIC_UMALQURA },
    { "japanese", CalendarType::JAPANESE },
    { "persion", CalendarType::PERSIAN },
};
using namespace OHOS::HiviewDFX;

I18nAddon::I18nAddon() : env_(nullptr), wrapper_(nullptr) {}

I18nAddon::~I18nAddon()
{
    napi_delete_reference(env_, wrapper_);
}

void I18nAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    if (nativeObject == nullptr) {
        return;
    }
    reinterpret_cast<I18nAddon *>(nativeObject)->~I18nAddon();
}

napi_value I18nAddon::Init(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_value util;
    status = napi_create_object(env, &util);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create util object at init");
        return nullptr;
    }
    napi_property_descriptor utilProperties[] = {
        DECLARE_NAPI_FUNCTION("unitConvert", UnitConvert),
    };
    status = napi_define_properties(env, util,
                                    sizeof(utilProperties) / sizeof(napi_property_descriptor),
                                    utilProperties);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to set properties of util at init");
        return nullptr;
    }
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSystemLanguages", GetSystemLanguages),
        DECLARE_NAPI_FUNCTION("getSystemCountries", GetSystemCountries),
        DECLARE_NAPI_FUNCTION("isSuggested", IsSuggested),
        DECLARE_NAPI_FUNCTION("getDisplayLanguage", GetDisplayLanguage),
        DECLARE_NAPI_FUNCTION("getDisplayCountry", GetDisplayCountry),
        DECLARE_NAPI_FUNCTION("getSystemLanguage", GetSystemLanguage),
        DECLARE_NAPI_FUNCTION("getSystemRegion", GetSystemRegion),
        DECLARE_NAPI_FUNCTION("getSystemLocale", GetSystemLocale),
        DECLARE_NAPI_FUNCTION("setSystemLanguage", SetSystemLanguage),
        DECLARE_NAPI_FUNCTION("setSystemRegion", SetSystemRegion),
        DECLARE_NAPI_FUNCTION("setSystemLocale", SetSystemLocale),
        DECLARE_NAPI_FUNCTION("getCalendar", GetCalendar),
        DECLARE_NAPI_FUNCTION("isRTL", IsRTL),
        DECLARE_NAPI_PROPERTY("Util", util),
        DECLARE_NAPI_FUNCTION("getLineInstance", GetLineInstance),
    };

    status = napi_define_properties(env, exports,
                                    sizeof(properties) / sizeof(napi_property_descriptor),
                                    properties);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to set properties at init");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::string &value)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        HiLog::Error(LABEL, "Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to get string item");
                return;
            }
            value = optionBuf.data();
        }
    }
}

void GetOptionMap(napi_env env, napi_value option, std::map<std::string, std::string> &map)
{
    if (option != nullptr) {
        size_t len;
        napi_get_value_string_utf8(env, option, nullptr, 0, &len);
        std::vector<char> styleBuf(len + 1);
        napi_status status = napi_get_value_string_utf8(env, option, styleBuf.data(), len + 1, &len);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to get string item");
            return;
        }
        map.insert(std::make_pair("unitDisplay", styleBuf.data()));
    }
}

napi_value I18nAddon::UnitConvert(napi_env env, napi_callback_info info)
{
    size_t argc = 5;
    napi_value argv[5] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    std::string fromUnit;
    GetOptionValue(env, argv[0], "unit", fromUnit);
    std::string fromMeasSys;
    GetOptionValue(env, argv[0], "measureSystem", fromMeasSys);
    std::string toUnit;
    GetOptionValue(env, argv[1], "unit", toUnit);
    std::string toMeasSys;
    GetOptionValue(env, argv[1], "measureSystem", toMeasSys);
    double number = 0;
    napi_get_value_double(env, argv[2], &number); // 2 is the index of value
    int convertStatus = Convert(number, fromUnit, fromMeasSys, toUnit, toMeasSys);
    size_t len;
    napi_get_value_string_utf8(env, argv[3], nullptr, 0, &len); // 3 is the index of value
    std::vector<char> localeBuf(len + 1);
    // 3 is the index of value
    status = napi_get_value_string_utf8(env, argv[3], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    std::vector<std::string> localeTags;
    localeTags.push_back(localeBuf.data());
    std::map<std::string, std::string> map = {};
    map.insert(std::make_pair("style", "unit"));
    if (convertStatus == 0) {
        HiLog::Error(LABEL, "Do not support the conversion");
        map.insert(std::make_pair("unit", fromUnit));
    } else {
        map.insert(std::make_pair("unit", toUnit));
    }
    // 4 is the index of value
    GetOptionMap(env, argv[4], map);
    auto numberFmt = std::make_unique<NumberFormat>(localeTags, map);
    std::string value = numberFmt->Format(number);
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemLanguages(napi_env env, napi_callback_info info)
{
    std::vector<std::string> systemLanguages;
    LocaleConfig::GetSystemLanguages(systemLanguages);
    napi_value result = nullptr;
    napi_status status = napi_create_array_with_length(env, systemLanguages.size(), &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create array");
        return nullptr;
    }
    for (size_t i = 0; i < systemLanguages.size(); i++) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, systemLanguages[i].c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, i, value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to set array item");
            return nullptr;
        }
    }
    return result;
}

napi_value I18nAddon::GetSystemCountries(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    std::vector<std::string> systemCountries;
    LocaleConfig::GetSystemCountries(systemCountries);
    napi_value result = nullptr;
    status = napi_create_array_with_length(env, systemCountries.size(), &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create array");
        return nullptr;
    }
    for (size_t i = 0; i < systemCountries.size(); i++) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, systemCountries[i].c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, i, value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to set array item");
            return nullptr;
        }
    }
    return result;
}

napi_value I18nAddon::GetSystemLanguage(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemLanguage();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemRegion(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemRegion();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemLocale(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemLocale();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDisplayLanguage(napi_env env, napi_callback_info info)
{
    // Need to get three parameters to get the display Language.
    size_t argc = 3;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    std::vector<char> displayLocaleBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[1], displayLocaleBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool sentenceCase = true;
    int sentenceCaseIndex = 2;
    if (argv[sentenceCaseIndex] != nullptr) {
        napi_get_value_bool(env, argv[sentenceCaseIndex], &sentenceCase);
    }

    std::string value = LocaleConfig::GetDisplayLanguage(localeBuf.data(), displayLocaleBuf.data(), sentenceCase);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDisplayCountry(napi_env env, napi_callback_info info)
{
    // Need to get three parameters to get the display country.
    size_t argc = 3;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    std::vector<char> displayLocaleBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[1], displayLocaleBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool sentenceCase = true;
    int sentenceCaseIndex = 2;
    if (argv[sentenceCaseIndex] != nullptr) {
        napi_get_value_bool(env, argv[sentenceCaseIndex], &sentenceCase);
    }
    std::string value = LocaleConfig::GetDisplayRegion(localeBuf.data(), displayLocaleBuf.data(), sentenceCase);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsSuggested(napi_env env, napi_callback_info info)
{
    // Need to get two parameters to check is suggested or not.
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> languageBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], languageBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool isSuggested = false;
    if (argv[1] != nullptr) {
        napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
        std::vector<char> regionBuf(len + 1);
        status = napi_get_value_string_utf8(env, argv[1], regionBuf.data(), len + 1, &len);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to get string item");
            return nullptr;
        }
        isSuggested = LocaleConfig::IsSuggested(languageBuf.data(), regionBuf.data());
    } else {
        isSuggested = LocaleConfig::IsSuggested(languageBuf.data());
    }
    napi_value result = nullptr;
    status = napi_get_boolean(env, isSuggested, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create case first boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetSystemLanguage(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> languageBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], languageBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool success = LocaleConfig::SetSystemLanguage(languageBuf.data());
    napi_value result = nullptr;
    status = napi_get_boolean(env, success, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create set system language boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetSystemRegion(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> regionBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], regionBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool success = LocaleConfig::SetSystemRegion(regionBuf.data());
    napi_value result = nullptr;
    status = napi_get_boolean(env, success, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create set system language boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetSystemLocale(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool success = LocaleConfig::SetSystemLocale(localeBuf.data());
    napi_value result = nullptr;
    status = napi_get_boolean(env, success, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create set system language boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsRTL(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool isRTL = LocaleConfig::IsRTL(localeBuf.data());
    napi_value result = nullptr;
    status = napi_get_boolean(env, isRTL, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "IsRTL failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::InitPhoneNumberFormat(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("isValidNumber", IsValidPhoneNumber),
        DECLARE_NAPI_FUNCTION("format", FormatPhoneNumber)
    };

    napi_value constructor;
    status = napi_define_class(env, "PhoneNumberFormat", NAPI_AUTO_LENGTH, PhoneNumberFormatConstructor, nullptr,
                               sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Define class failed when InitPhoneNumberFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "PhoneNumberFormat", constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Set property failed when InitPhoneNumberFormat");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
                    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        HiLog::Error(LABEL, "Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len = 0;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            map.insert(make_pair(optionName, optionBuf.data()));
        }
    }
}

napi_value I18nAddon::PhoneNumberFormatConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len = 0;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get country tag length failed");
        return nullptr;
    }

    std::vector<char> country (len + 1);
    status = napi_get_value_string_utf8(env, argv[0], country.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get country tag failed");
        return nullptr;
    }

    std::map<std::string, std::string> options;
    GetOptionValue(env, argv[1], "type", options);

    std::unique_ptr<I18nAddon> obj = std::make_unique<I18nAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create I18nAddon failed");
        return nullptr;
    }

    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       I18nAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap I18nAddon failed");
        return nullptr;
    }

    if (!obj->InitPhoneNumberFormatContext(env, info, country.data(), options)) {
        return nullptr;
    }

    obj.release();

    return thisVar;
}

bool I18nAddon::InitPhoneNumberFormatContext(napi_env env, napi_callback_info info, const std::string &country,
                                             const std::map<std::string, std::string> &options)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get global failed");
        return false;
    }
    env_ = env;
    phonenumberfmt_ = std::make_unique<PhoneNumberFormat>(country, options);

    return phonenumberfmt_ != nullptr;
}

napi_value I18nAddon::IsValidPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number failed");
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->phonenumberfmt_ == nullptr) {
        HiLog::Error(LABEL, "GetPhoneNumberFormat object failed");
        return nullptr;
    }

    bool isValid = obj->phonenumberfmt_->isValidPhoneNumber(buf.data());

    napi_value result = nullptr;
    status = napi_get_boolean(env, isValid, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create boolean failed");
        return nullptr;
    }

    return result;
}

napi_value I18nAddon::FormatPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number failed");
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->phonenumberfmt_ == nullptr) {
        HiLog::Error(LABEL, "Get PhoneNumberFormat object failed");
        return nullptr;
    }

    std::string formattedPhoneNumber = obj->phonenumberfmt_->format(buf.data());

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, formattedPhoneNumber.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create format phone number failed");
        return nullptr;
    }
    return result;
}

std::string I18nAddon::GetString(napi_env &env, napi_value &value, int32_t &code)
{
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get string failed");
        code = 1;
        return "";
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, value, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create string failed");
        code = 1;
        return "";
    }
    return buf.data();
}

napi_value I18nAddon::InitI18nCalendar(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setTime", SetTime),
        DECLARE_NAPI_FUNCTION("set", Set),
        DECLARE_NAPI_FUNCTION("getTimeZone", GetTimeZone),
        DECLARE_NAPI_FUNCTION("setTimeZone", SetTimeZone),
        DECLARE_NAPI_FUNCTION("getFirstDayOfWeek", GetFirstDayOfWeek),
        DECLARE_NAPI_FUNCTION("setFirstDayOfWeek", SetFirstDayOfWeek),
        DECLARE_NAPI_FUNCTION("getMinimalDaysInFirstWeek", GetMinimalDaysInFirstWeek),
        DECLARE_NAPI_FUNCTION("setMinimalDaysInFirstWeek", SetMinimalDaysInFirstWeek),
        DECLARE_NAPI_FUNCTION("get", Get),
        DECLARE_NAPI_FUNCTION("getDisplayName", GetDisplayName),
        DECLARE_NAPI_FUNCTION("isWeekend", IsWeekend)
    };
    napi_value constructor = nullptr;
    status = napi_define_class(env, "I18nCalendar", NAPI_AUTO_LENGTH, CalendarConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to define class at Init");
        return nullptr;
    }
    g_constructor = new (std::nothrow) napi_ref;
    if (g_constructor == nullptr) {
        HiLog::Error(LABEL, "Failed to create ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::CalendarConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    argv[0] = nullptr;
    argv[1] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string localeTag = GetString(env, argv[0], code);
    if (code != 0) {
        return nullptr;
    }
    CalendarType type = GetCalendarType(env, argv[1]);
    std::unique_ptr<I18nAddon> obj = std::make_unique<I18nAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create I18nAddon failed");
        return nullptr;
    }
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap II18nAddon failed");
        return nullptr;
    }
    if (!obj->InitCalendarContext(env, info, localeTag, type)) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

CalendarType I18nAddon::GetCalendarType(napi_env env, napi_value value)
{
    CalendarType type = CalendarType::UNDEFINED;
    if (value != nullptr) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, value, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return type;
        }
        int32_t code = 0;
        std::string calendarType = GetString(env, value, code);
        if (code != 0) {
            return type;
        }
        if (g_typeMap.find(calendarType) != g_typeMap.end()) {
            type = g_typeMap[calendarType];
        }
    }
    return type;
}

bool I18nAddon::InitCalendarContext(napi_env env, napi_callback_info info, const std::string &localeTag,
    CalendarType type)
{
    calendar_ = std::make_unique<I18nCalendar>(localeTag, type);
    return calendar_ != nullptr;
}

napi_value I18nAddon::GetCalendar(napi_env env, napi_callback_info info)
{
    size_t argc = 2; // retrieve 2 arguments
    napi_value argv[2] = { 0 };
    argv[0] = nullptr;
    argv[1] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create reference at GetCalendar");
        return nullptr;
    }
    if (argv[1] == nullptr) {
        status = napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, argv + 1);
        if (status != napi_ok) {
            return nullptr;
        }
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 2, argv, &result); // 2 arguments
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get calendar create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDate(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return nullptr;
    }
    napi_value funcGetDateInfo = nullptr;
    napi_status status = napi_get_named_property(env, value, "valueOf", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get method valueOf failed");
        return nullptr;
    }
    napi_value ret_value = nullptr;
    status = napi_call_function(env, value, funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get milliseconds failed");
        return nullptr;
    }
    return ret_value;
}

void I18nAddon::SetMilliseconds(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return;
    }
    double milliseconds = 0;
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return;
    }
    napi_status status = napi_get_value_double(env, value, &milliseconds);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Retrieve milliseconds failed");
        return;
    }
    if (calendar_ != nullptr) {
        calendar_->SetTime(milliseconds);
    }
}

napi_value I18nAddon::Set(napi_env env, napi_callback_info info)
{
    size_t argc = 6; // Set may have 6 arguments
    napi_value argv[6] = { 0 };
    for (size_t i = 0; i < argc; ++i) {
        argv[i] = nullptr;
    }
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status status = napi_ok;
    int32_t times[3] = { 0 }; // There are at least 3 arguments.
    for (int i = 0; i < 3; ++i) { // There are at least 3 arguments.
        napi_typeof(env, argv[i], &valueType);
        if (valueType != napi_valuetype::napi_number) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return nullptr;
        }
        status = napi_get_value_int32(env, argv[i], times + i);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Retrieve time value failed");
            return nullptr;
        }
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->Set(times[0], times[1], times[2]); // 2 is the index of date
    obj->SetField(env, argv[3], UCalendarDateFields::UCAL_HOUR_OF_DAY); // 3 is the index of hour
    obj->SetField(env, argv[4], UCalendarDateFields::UCAL_MINUTE); // 4 is the index of minute
    obj->SetField(env, argv[5], UCalendarDateFields::UCAL_SECOND); // 5 is the index of second
    return nullptr;
}

napi_value I18nAddon::SetTime(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argv[0] == nullptr) {
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    napi_valuetype type = napi_valuetype::napi_undefined;
    status = napi_typeof(env, argv[0], &type);
    if (status != napi_ok) {
        return nullptr;
    }
    if (type == napi_valuetype::napi_number) {
        obj->SetMilliseconds(env, argv[0]);
        return nullptr;
    } else {
        napi_value val = GetDate(env, argv[0]);
        if (val == nullptr) {
            return nullptr;
        }
        obj->SetMilliseconds(env, val);
        return nullptr;
    }
}

void I18nAddon::SetField(napi_env env, napi_value value, UCalendarDateFields field)
{
    if (value == nullptr) {
        return;
    }
    int32_t val = 0;
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return;
    }
    napi_status status = napi_get_value_int32(env, value, &val);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Retrieve field failed");
        return;
    }
    if (calendar_ != nullptr) {
        calendar_->Set(field, val);
    }
}

napi_value I18nAddon::SetTimeZone(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get timezone length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get timezone failed");
        return nullptr;
    }
    std::string timezone(buf.data());
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->SetTimeZone(timezone);
    return nullptr;
}

napi_value I18nAddon::GetTimeZone(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    std::string temp = obj->calendar_->GetTimeZone();
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, temp.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create timezone string failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetFirstDayOfWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    int32_t temp = obj->calendar_->GetFirstDayOfWeek();
    napi_value result = nullptr;
    status = napi_create_int32(env, temp, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32 failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetMinimalDaysInFirstWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    int32_t temp = obj->calendar_->GetMinimalDaysInFirstWeek();
    napi_value result = nullptr;
    status = napi_create_int32(env, temp, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32 failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetFirstDayOfWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get int32 failed");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->SetFirstDayOfWeek(value);
    return nullptr;
}

napi_value I18nAddon::SetMinimalDaysInFirstWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get int32 failed");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->SetMinimalDaysInFirstWeek(value);
    return nullptr;
}

napi_value I18nAddon::Get(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get field length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get field failed");
        return nullptr;
    }
    std::string field(buf.data());
    if (g_fieldsMap.find(field) == g_fieldsMap.end()) {
        HiLog::Error(LABEL, "Invalid field");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    int32_t value = obj->calendar_->Get(g_fieldsMap[field]);
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32 failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsWeekend(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    bool isWeekEnd = false;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    do {
        if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
            HiLog::Error(LABEL, "Get calendar object failed");
            break;
        }
        if (argv[0] == nullptr) {
            isWeekEnd = obj->calendar_->IsWeekend();
        } else {
            napi_value funcGetDateInfo = nullptr;
            status = napi_get_named_property(env, argv[0], "valueOf", &funcGetDateInfo);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Get method now failed");
                break;
            }
            napi_value value = nullptr;
            status = napi_call_function(env, argv[0], funcGetDateInfo, 0, nullptr, &value);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Get milliseconds failed");
                break;
            }
            double milliseconds = 0;
            status = napi_get_value_double(env, value, &milliseconds);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Retrieve milliseconds failed");
                break;
            }
            UErrorCode error = U_ZERO_ERROR;
            isWeekEnd = obj->calendar_->IsWeekend(milliseconds, error);
        }
    } while (false);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isWeekEnd, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create boolean failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDisplayName(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string localeTag = GetString(env, argv[0], code);
    if (code != 0) {
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->calendar_ == nullptr) {
        HiLog::Error(LABEL, "Get calendar object failed");
        return nullptr;
    }
    if (obj->calendar_ == nullptr) {
        return nullptr;
    }
    std::string name = obj->calendar_->GetDisplayName(localeTag);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create calendar name string failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::BreakIteratorConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string localeTag = GetString(env, argv[0], code);
    if (code != 0) {
        return nullptr;
    }
    std::unique_ptr<I18nAddon> obj = std::make_unique<I18nAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create I18nAddon failed");
        return nullptr;
    }
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap II18nAddon failed");
        return nullptr;
    }
    obj->brkiter_ = std::make_unique<I18nBreakIterator>(localeTag);
    if (obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Wrap BreakIterator failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

napi_value I18nAddon::InitBreakIterator(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("current", Current),
        DECLARE_NAPI_FUNCTION("first", First),
        DECLARE_NAPI_FUNCTION("last", Last),
        DECLARE_NAPI_FUNCTION("next", Next),
        DECLARE_NAPI_FUNCTION("previous", Previous),
        DECLARE_NAPI_FUNCTION("setLineBreakText", SetText),
        DECLARE_NAPI_FUNCTION("following", Following),
        DECLARE_NAPI_FUNCTION("getLineBreakText", GetText),
        DECLARE_NAPI_FUNCTION("isBoundary", IsBoundary),
    };
    napi_value constructor = nullptr;
    status = napi_define_class(env, "BreakIterator", NAPI_AUTO_LENGTH, BreakIteratorConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to define class BreakIterator at Init");
        return nullptr;
    }
    g_BrkConstructor = new (std::nothrow) napi_ref;
    if (g_BrkConstructor == nullptr) {
        HiLog::Error(LABEL, "Failed to create brkiterator ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_BrkConstructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create reference g_BrkConstructor at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::GetLineInstance(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, *g_BrkConstructor, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create reference at GetLineInstance");
        return nullptr;
    }
    if (argv[0] == nullptr) {
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 1, argv, &result); // 1 arguments
    if (status != napi_ok) {
        HiLog::Error(LABEL, "GetLineInstance create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Current(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->current();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::First(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->first();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Last(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->last();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Previous(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->previous();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Next(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    int value = 1;
    if (argv[0] != nullptr) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_valuetype::napi_number) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return nullptr;
        }
        status = napi_get_value_int32(env, argv[0], &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Retrieve next value failed");
            return nullptr;
        }
    }
    value = obj->brkiter_->next(value);
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetText(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    if (argv[0] == nullptr) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get field length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get string value failed");
        return nullptr;
    }
    obj->brkiter_->setText(buf.data());
    return nullptr;
}

napi_value I18nAddon::GetText(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    napi_value value = nullptr;
    std::string temp;
    obj->brkiter_->getText(temp);
    status = napi_create_string_utf8(env, temp.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get field length failed");
        return nullptr;
    }
    return value;
}

napi_value I18nAddon::Following(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    if (argv[0] == nullptr) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int value;
    status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Retrieve following value failed");
        return nullptr;
    }
    value = obj->brkiter_->following(value);
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsBoundary(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->brkiter_ == nullptr) {
        HiLog::Error(LABEL, "Get BreakIterator object failed");
        return nullptr;
    }
    if (argv[0] == nullptr) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    int value;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Retrieve following value failed");
        return nullptr;
    }
    bool boundary = obj->brkiter_->isBoundary(value);
    napi_value result = nullptr;
    status = napi_get_boolean(env, boundary, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create boolean failed");
        return nullptr;
    }
    return result;
}


napi_value Init(napi_env env, napi_value exports)
{
    napi_value val = I18nAddon::Init(env, exports);
    val = I18nAddon::InitPhoneNumberFormat(env, val);
    val = I18nAddon::InitBreakIterator(env, val);
    return I18nAddon::InitI18nCalendar(env, val);
}

static napi_module g_i18nModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "i18n",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void I18nRegister()
{
    napi_module_register(&g_i18nModule);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS