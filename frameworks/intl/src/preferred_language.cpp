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
#include <regex>
#include "bundle_mgr_client.h"
#include "hap_resource.h"
#include "hilog/log.h"
#include "ipc_skeleton.h"
#include "locale_config.h"
#include "locale_info.h"
#include "parameter.h"
#include "preferred_language.h"

namespace OHOS {
namespace Global {
namespace I18n {
const char *PreferredLanguage::RESOURCE_PATH_HEAD = "/data/accounts/account_0/applications/";
const char *PreferredLanguage::RESOURCE_PATH_TAILOR = "/resources.index";
const char *PreferredLanguage::RESOURCE_PATH_SPLITOR = "/";
const char *PreferredLanguage::PREFERRED_LANGUAGES = "persist.sys.preferredLanguages";
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "LocaleConfig" };

bool PreferredLanguage::AddPreferredLanguageExist(std::vector<std::string> &preferredLanguageList, int languageIdx,
    int index, const std::string& language)
{
    if (languageIdx == index) {
        return false;
    }
    if (languageIdx < index) {
        for (int i = languageIdx; i < index; i++) {
            preferredLanguageList[i] = preferredLanguageList[i+1];
        }
    }
    if (languageIdx > index) {
        for (int i = languageIdx; i > index; i--) {
            preferredLanguageList[i] = preferredLanguageList[i-1];
        }
    }
    preferredLanguageList[index] = language;
    if (languageIdx == 0 || index == 0) {
        bool status = LocaleConfig::SetSystemLanguage(preferredLanguageList[0]);
        if (!status) {
            return false;
        }
    }
    return true;
}

bool PreferredLanguage::AddPreferredLanguageNonExist(std::vector<std::string> &preferredLanguageList, int index,
    const std::string& language)
{
    preferredLanguageList.insert(preferredLanguageList.begin() + index, language);
    if (index == 0) {
        bool status = LocaleConfig::SetSystemLanguage(preferredLanguageList[0]);
        if (!status) {
            return false;
        }
    }
    return true;
}

bool PreferredLanguage::AddPreferredLanguage(const std::string &language, int index)
{
    if (!IsValidTag(language)) {
        return false;
    }
    std::vector<std::string> preferredLanguageList = GetPreferredLanguageList();
    int idx = index;
    if (index < 0) {
        idx = 0;
    }
    if (index >= (int)preferredLanguageList.size()) {
        idx = (int)preferredLanguageList.size();
    }
    int languageIdx = -1;
    for (size_t i = 0; i < preferredLanguageList.size(); i++) {
        if (preferredLanguageList[i] == language) {
            languageIdx = (int)i;
            break;
        }
    }
    bool status = true;
    if (languageIdx == -1) {
        status = AddPreferredLanguageNonExist(preferredLanguageList, idx, language);
    } else {
        if (idx == (int)preferredLanguageList.size()) {
            idx -= 1;
        }
        status = AddPreferredLanguageExist(preferredLanguageList, languageIdx, idx, language);
    }
    if (!status) {
        return false;
    }
    std::string result = "";
    for (size_t i = 0; i < preferredLanguageList.size(); i++) {
        result += preferredLanguageList[i];
        result += ";";
    }
    result.pop_back();
    if (result.length() > CONFIG_LEN) {
        return false;
    }
    status = SetParameter(PREFERRED_LANGUAGES, result.data()) == 0;
    return status;
}

bool PreferredLanguage::RemovePreferredLanguage(int index)
{
    std::vector<std::string> preferredLanguageList = GetPreferredLanguageList();
    int idx = index;
    if (index < 0) {
        idx = 0;
    }
    if (index >= (int)preferredLanguageList.size()) {
        idx = (int)preferredLanguageList.size() - 1;
    }
    if (preferredLanguageList.size() == 1) {
        return false;
    }
    std::vector<std::string>::iterator it = preferredLanguageList.begin();
    preferredLanguageList.erase(it + index);
    bool status = false;
    if (index == 0) {
        status = LocaleConfig::SetSystemLanguage(preferredLanguageList[0]);
        if (!status) {
            return false;
        }
    }
    std::string result = "";
    for (it = preferredLanguageList.begin(); it != preferredLanguageList.end(); ++it) {
        result += *it;
        result += ";";
    }
    result.pop_back();
    status = SetParameter(PREFERRED_LANGUAGES, result.data()) == 0;
    return status;
}

std::vector<std::string> PreferredLanguage::GetPreferredLanguageList()
{
    char preferredLanguageValue[CONFIG_LEN];
    GetParameter(PREFERRED_LANGUAGES, "", preferredLanguageValue, CONFIG_LEN);
    std::string systemLanguage = LocaleConfig::GetSystemLanguage();
    std::vector<std::string> list;
    Split(preferredLanguageValue, ";", list);
    if (list.size() == 0) {
        if (systemLanguage != "") {
            list.push_back(systemLanguage);
        }
        return list;
    }
    if (list[0] == systemLanguage || systemLanguage == "") {
        return list;
    }
    int systemLanguageIdx = -1;
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i] == systemLanguage) {
            systemLanguageIdx = (int)i;
        }
    }
    if (systemLanguageIdx == -1) {
        list.insert(list.begin(), systemLanguage);
    } else {
        for (size_t i = systemLanguageIdx; i > 0; i--) {
            list[i] = list[i-1];
        }
        list[0] = systemLanguage;
    }
    return list;
}

std::set<std::string> PreferredLanguage::GetResources()
{
    pid_t uid = OHOS::IPCSkeleton::GetCallingUid();
    std::string bundleName = "";
    OHOS::AppExecFwk::BundleMgrClient client;
    bool status = client.GetBundleNameForUid(uid, bundleName);
    if (!status) {
        HiviewDFX::HiLog::Error(LABEL, "Failed to get bundleName");
    }
    const std::string resourcePath = RESOURCE_PATH_HEAD + bundleName + RESOURCE_PATH_SPLITOR + bundleName +
        RESOURCE_PATH_TAILOR;
    const OHOS::Global::Resource::HapResource *resource =
        OHOS::Global::Resource::HapResource::LoadFromIndex(resourcePath.c_str(), nullptr);
    const std::vector<std::string> qualifiers = resource->GetQualifiers();
    std::set<std::string> result;
    std::regex languagePattern("type:0.*str:([a-z]{2})");
    std::regex countryPattern("type:1.*str:([A-Z]{2})");
    for (size_t i = 0; i < qualifiers.size(); i++) {
        std::smatch match;
        bool found = regex_search(qualifiers[i], match, languagePattern);
        if (!found) {
            continue;
        }
        std::string locale = match.str(1);
        found = regex_search(qualifiers[i], match, countryPattern);
        if (found) {
            locale += "-";
            locale += match.str(1);
        }
        result.insert(locale);
    }
    return result;
}

bool PreferredLanguage::IsMatched(const std::string& preferred, const std::string& resource)
{
    LocaleInfo preferredLocaleInfo(preferred);
    LocaleInfo resourceLocaleInfo(resource);
    if (preferred == resource) {
        return true;
    }
    if (preferredLocaleInfo.GetLanguage() != resourceLocaleInfo.GetLanguage()) {
        return false;
    }
    LocaleInfo maximizedResourceLocale(resourceLocaleInfo.Maximize());
    std::string resourceScript = maximizedResourceLocale.GetScript();
    if (resourceScript == "") {
        std::string resourceCountry = maximizedResourceLocale.GetRegion();
        if (resourceCountry == "") {
            return true;
        }
        std::string preferredCountry = preferredLocaleInfo.GetRegion();
        if (resourceCountry == preferredCountry) {
            return true;
        }
        return false;
    }
    LocaleInfo maximizedPreferredLocale(preferredLocaleInfo.Maximize());
    std::string preferredScript = maximizedResourceLocale.GetScript();
    if (resourceScript == preferredScript) {
        return true;
    }
    return false;
}

std::string PreferredLanguage::GetFirstPreferredLanguage()
{
    std::vector<std::string> preferredLanguageList = GetPreferredLanguageList();
    std::set<std::string> resources = GetResources();
    int minmumMatchedIdx = -1;
    for (size_t i = 0; i < preferredLanguageList.size(); i++) {
        for (std::set<std::string>::iterator it = resources.begin(); it != resources.end(); ++it) {
            std::string preferredLanguage = preferredLanguageList[i];
            if (preferredLanguage == "en-Qaag") {
                preferredLanguage = "en-Latn";
            }
            if (IsMatched(preferredLanguage, *it)) {
                minmumMatchedIdx = (int)i;
                break;
            }
        }
    }
    if (minmumMatchedIdx != -1) {
        return preferredLanguageList[minmumMatchedIdx];
    }
    return preferredLanguageList[0];
}

std::string PreferredLanguage::GetPreferredLocale()
{
    std::string systemLocale = LocaleConfig::GetSystemLocale();
    LocaleInfo systemLocaleInfo(systemLocale);
    std::string systemRegion = systemLocaleInfo.GetRegion();
    std::string preferredLanguageLocale = GetFirstPreferredLanguage();
    LocaleInfo preferredLanguageLocaleInfo(preferredLanguageLocale);
    std::string preferredLanguage = preferredLanguageLocaleInfo.GetLanguage();
    std::string preferredLocale = preferredLanguage + "-" + systemRegion;
    return preferredLocale;
}

bool PreferredLanguage::IsValidLanguage(const std::string &language)
{
    std::string::size_type size = language.size();
    if ((size != LANGUAGE_LEN) && (size != LANGUAGE_LEN + 1)) {
        return false;
    }
    for (size_t i = 0; i < size; ++i) {
        if ((language[i] > 'z') || (language[i] < 'a')) {
            return false;
        }
    }
    return true;
}

bool PreferredLanguage::IsValidTag(const std::string &tag)
{
    if (tag.size() == 0) {
        return false;
    }
    std::vector<std::string> splits;
    Split(tag, "-", splits);
    if (!IsValidLanguage(splits[0])) {
        return false;
    }
    return true;
}

void PreferredLanguage::Split(const std::string &src, const std::string &sep, std::vector<std::string> &dest)
{
    std::string::size_type begin = 0;
    std::string::size_type end = src.find(sep);
    while (end != std::string::npos) {
        dest.push_back(src.substr(begin, end - begin));
        begin = end + sep.size();
        end = src.find(sep, begin);
    }
    if (begin != src.size()) {
        dest.push_back(src.substr(begin));
    }
}
}
}
}