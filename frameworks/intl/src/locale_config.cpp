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
#include <algorithm>
#include <memory.h>
#include <unordered_set>
#include "locale_config.h"
#include "core_manager.h"
#include "libxml/parser.h"
#include "locale_info.h"
#include "localebuilder.h"
#include "locid.h"
#include "ohos/init_data.h"
#include "parameter.h"
#include "sim_card_manager.h"
#include "string_ex.h"
#include "ucase.h"
#include "unistr.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;

const char *LocaleConfig::LANGUAGE_KEY = "hm.sys.language";
const char *LocaleConfig::LOCALE_KEY = "hm.sys.locale";
const char *LocaleConfig::DEFAULT_LOCALE = "zh-Hans-CN";
const char *LocaleConfig::DEFAULT_LANGUAGE = "zh-Hans";
const char *LocaleConfig::DEFAULT_REGION = "CN";
const char *LocaleConfig::SUPPORTED_LOCALES_NAME = "supported_locales";
const char *LocaleConfig::SUPPORTED_REGIONS_NAME = "supported_regions";
const char *LocaleConfig::WHITE_LANGUAGE_NAME = "white_language";
const char *LocaleConfig::SUPPORTED_LOCALES_PATH = "/system/usr/ohos_locale_config/supported_locales.xml";
const char *LocaleConfig::SUPPORTED_REGIONS_PATH = "/system/usr/ohos_locale_config/supported_regions.xml";
const char *LocaleConfig::WHILTE_LANGUAGES_PATH = "/system/usr/ohos_locale_config/white_languages.xml";
unordered_set<string> LocaleConfig::forbiddenRegions;
unordered_set<string> LocaleConfig::supportedLocales;
unordered_set<string> LocaleConfig::supportedRegions;
unordered_set<string> LocaleConfig::whiteLanguages;

bool LocaleConfig::listsInitialized = LocaleConfig::InitializeLists();

string LocaleConfig::GetSystemLanguage()
{
    char value[CONFIG_LEN];
    int code = GetParameter(LANGUAGE_KEY, "", value, CONFIG_LEN);
    if (code > 0) {
        return value;
    }
    return DEFAULT_LANGUAGE;
}

string LocaleConfig::GetSystemRegion()
{
    string locale = GetSystemLocale();
    char value[CONFIG_LEN];
    int code = GetParameter(LOCALE_KEY, "", value, CONFIG_LEN);
    if (code > 0) {
        UErrorCode status = U_ZERO_ERROR;
        icu::Locale origin = icu::Locale::forLanguageTag(value, status);
        if (status == U_ZERO_ERROR) {
            return origin.getCountry();
        }
    }
    return DEFAULT_REGION;
}

string LocaleConfig::GetSystemLocale()
{
    char value[CONFIG_LEN];
    int code = GetParameter(LOCALE_KEY, "", value, CONFIG_LEN);
    if (code > 0) {
        return value;
    }
    return DEFAULT_LOCALE;
}

bool LocaleConfig::SetSystemLanguage(const string &language)
{
    if ((language != "") && !IsValidTag(language)) {
        return false;
    }
    return SetParameter(LANGUAGE_KEY, language.data()) == 0;
}

bool LocaleConfig::SetSystemRegion(const string &region)
{
    if ((region != "") && !IsValidRegion(region)) {
        return false;
    }
    char value[CONFIG_LEN];
    int code = GetParameter(LOCALE_KEY, "", value, CONFIG_LEN);
    string newLocale;
    if (code > 0) {
        newLocale = GetRegionChangeLocale(value, region);
        if (newLocale == "") {
            return false;
        }
    } else {
        icu::Locale temp("", region.c_str());
        UErrorCode status = U_ZERO_ERROR;
        temp.addLikelySubtags(status);
        if (status != U_ZERO_ERROR) {
            return false;
        }
        newLocale = temp.toLanguageTag<string>(status);
        if (status != U_ZERO_ERROR) {
            return false;
        }
    }
    return SetParameter(LOCALE_KEY, newLocale.data()) == 0;
}

bool LocaleConfig::SetSystemLocale(const string &locale)
{
    if ((locale != "") && !IsValidTag(locale)) {
        return false;
    }
    return SetParameter(LOCALE_KEY, locale.data()) == 0;
}

bool LocaleConfig::IsValidLanguage(const string &language)
{
    const char * const *langs = uloc_getISOLanguages();
    uint32_t i = 0;
    while (*(langs + i) != nullptr) {
        if (language == *(langs + i)) {
            return true;
        }
        ++i;
    }
    return false;
}

bool LocaleConfig::IsValidScript(const string &script)
{
    string::size_type size = script.size();
    if (size != LocaleInfo::SCRIPT_LEN) {
        return false;
    }
    char first = script[0];
    if ((first < 'A') || (first > 'Z')) {
        return false;
    }
    for (string::size_type i = 1; i < LocaleInfo::SCRIPT_LEN; ++i) {
        if ((script[i] > 'z') || (script[i] < 'a')) {
            return false;
        }
    }
    return true;
}

bool LocaleConfig::IsValidRegion(const string &region)
{
    const char * const *countries = uloc_getISOCountries();
    uint32_t i = 0;
    while (*(countries + i) != nullptr) {
        if (region == *(countries + i)) {
            return true;
        }
        ++i;
    }
    return false;
}

bool LocaleConfig::IsValidTag(const string &tag)
{
    vector<string> splits;
    Split(tag, "-", splits);
    if (!IsValidLanguage(splits[0])) {
        return false;
    }
    return true;
}

void LocaleConfig::Split(const string &src, const string &sep, vector<string> &dest)
{
    string::size_type begin = 0;
    string::size_type end = src.find(sep);
    while (end != string::npos) {
        dest.push_back(src.substr(begin, end - begin));
        begin = end + sep.size();
        end = src.find(sep, begin);
    }
    if (begin != src.size()) {
        dest.push_back(src.substr(begin));
    }
}

void LocaleConfig::GetSystemLanguages(vector<string> &ret)
{
    const unordered_set<string> &supported = GetSupportedLocales();
    for (auto item : supported) {
        if (supported.find(item) != supported.end()) {
            ret.push_back(item);
        }
    }
}

const unordered_set<string>& LocaleConfig::GetSupportedLocales()
{
    return supportedLocales;
}

const unordered_set<string>& LocaleConfig::GetSupportedRegions()
{
    return supportedRegions;
}

void LocaleConfig::GetSystemCountries(vector<string> &ret)
{
    for (auto item : supportedRegions) {
        ret.push_back(item);
    }
}

bool LocaleConfig::IsSuggested(const string &language)
{
    unordered_set<string> relatedLocales;
    GetRelatedLocales(relatedLocales);
    return relatedLocales.find(language) != relatedLocales.end();
}

bool LocaleConfig::IsSuggested(const std::string &language, const std::string &region)
{
    unordered_set<string> relatedLocales;
    GetRelatedLocales(relatedLocales);
    if (relatedLocales.find(language) == relatedLocales.end()) {
        return false;
    }
    string temp = GetRegionChangeLocale(language, region);
    return relatedLocales.find(temp) != relatedLocales.end();
}

void LocaleConfig::GetRelatedLocales(unordered_set<string> &relatedLocales)
{
    vector<string> simCountries;
    GetCountriesFromSim(simCountries);
    const unordered_set<string> &regions = GetSupportedRegions();
    for (auto iter = simCountries.begin(); iter != simCountries.end();) {
        if (regions.find(*iter) == regions.end()) {
            iter = simCountries.erase(iter);
        } else {
            ++iter;
        }
    }
    const unordered_set<string> &locales = GetSupportedLocales();
    for (auto item : locales) {
        if (whiteLanguages.find(item) == whiteLanguages.end()) {
            continue;
        }
        UErrorCode status = U_ZERO_ERROR;
        icu::Locale locale = icu::Locale::forLanguageTag(item, status);
        if (status != U_ZERO_ERROR) {
            continue;
        }
        const char *region = locale.getCountry();
        if (region == nullptr) {
            continue;
        }
        if (find(simCountries.begin(), simCountries.end(), region) != simCountries.end()) {
            relatedLocales.insert(item);
        }
    }
}

void LocaleConfig::GetCountriesFromSim(vector<string> &simCountries)
{
    simCountries.push_back(GetSystemRegion());
    Telephony::SimCardManager simCardManager;
    simCardManager.ConnectService();
    simCountries.push_back(Str16ToStr8(simCardManager.GetIsoCountryCodeForSim(0)));
}

void LocaleConfig::GetListFromFile(const char *path, const char *resourceName, unordered_set<string> &ret)
{
    xmlKeepBlanksDefault(0);
    if (path == nullptr) {
        return;
    }
    xmlDocPtr doc = xmlParseFile(path);
    if (doc == nullptr) {
        return;
    }
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (cur == nullptr || (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar *>(resourceName))) != 0) {
        return;
    }
    cur = cur->xmlChildrenNode;
    xmlChar *content;
    while (cur != nullptr) {
        content = xmlNodeGetContent(cur);
        ret.insert(reinterpret_cast<const char*>(content));
        xmlFree(content);
        cur = cur->next;
    }
    xmlFreeDoc(doc);
}

const unordered_set<string>& LocaleConfig::GetForbiddenRegions()
{
    return forbiddenRegions;
}

bool LocaleConfig::InitializeLists()
{
    SetHwIcuDirectory();
    GetListFromFile(SUPPORTED_LOCALES_PATH, SUPPORTED_LOCALES_NAME, supportedLocales);
    for (auto iter = supportedLocales.begin(); iter != supportedLocales.end();) {
        bool find = false;
        for (auto item : forbiddenRegions) {
            if (iter->find(item)) {
                find = true;
                break;
            }
        }
        if (find) {
            iter = supportedLocales.erase(iter);
        } else {
            ++iter;
        }
    }
    GetListFromFile(SUPPORTED_REGIONS_PATH, SUPPORTED_REGIONS_NAME, supportedRegions);
    for (auto iter = supportedRegions.begin(); iter != supportedRegions.end();) {
        if (forbiddenRegions.find(*iter) != forbiddenRegions.end()) {
            iter = supportedRegions.erase(iter);
        } else {
            ++iter;
        }
    }
    GetListFromFile(WHILTE_LANGUAGES_PATH, WHITE_LANGUAGE_NAME, whiteLanguages);
    return true;
}

string LocaleConfig::GetRegionChangeLocale(const string &languageTag, const string &region)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale origin = icu::Locale::forLanguageTag(languageTag, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::LocaleBuilder builder = icu::LocaleBuilder().setLanguage(origin.getLanguage()).
        setScript(origin.getScript()).setRegion(region);
    icu::Locale temp = builder.setExtension('u', "").build(status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    string ret = temp.toLanguageTag<string>(status);
    return (status != U_ZERO_ERROR) ? "" : ret;
}

string LocaleConfig::GetDisplayLanguage(const string &language, const string &displayLocale, bool sentenceCase)
{
    UErrorCode status = U_ZERO_ERROR;
    const icu::Locale originLocale = icu::Locale::forLanguageTag(language, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    const icu::Locale locale = icu::Locale::forLanguageTag(displayLocale, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::UnicodeString displayLang;
    originLocale.getDisplayLanguage(locale, displayLang);
    if (sentenceCase) {
        UChar ch = ucase_toupper(displayLang.char32At(0));
        displayLang.replace(0, 1, ch);
    }
    string temp;
    displayLang.toUTF8String(temp);
    return temp;
}

string LocaleConfig::GetDisplayRegion(const string &region, const string &displayLocale, bool sentenceCase)
{
    UErrorCode status = U_ZERO_ERROR;
    const icu::Locale originLocale = icu::Locale::forLanguageTag(region, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::Locale locale = icu::Locale::forLanguageTag(displayLocale, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::UnicodeString displayRegion;
    originLocale.getDisplayCountry(locale, displayRegion);
    if (sentenceCase) {
        UChar ch = ucase_toupper(displayRegion.char32At(0));
        displayRegion.replace(0, 1, ch);
    }
    string temp;
    displayRegion.toUTF8String(temp);
    return temp;
}
} // I18n
} // Global
} // OHOS
