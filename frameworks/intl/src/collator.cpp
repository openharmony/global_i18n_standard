//
// Created by s00619675 on 2021/10/25.
//

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
#include "collator.h"

#include <stringpiece.h>

#include "unicode/ucol.h"
#include "unicode/errorcode.h"
#include "unicode/uloc.h"
#include "locale_config.h"

namespace OHOS {
namespace Global {
namespace I18n {
std::string Collator::ParseOption(std::map<std::string, std::string> &options, const std::string &key)
{
    std::map<std::string, std::string>::iterator it = options.find(key);
    if (it != options.end()) {
        return it->second;
    } else {
        return "";
    }
}

void Collator::ParseAllOptions(std::map<std::string, std::string> &options)
{
    localeMatcher = ParseOption(options, "localeMatcher");
    if (localeMatcher == "") {
        localeMatcher = "best fit";
    }

    usage = ParseOption(options, "usage");
    if (usage == "") {
        usage = "sort";
    }

    sensitivity = ParseOption(options, "sensitivity");
    if (sensitivity == "") {
        sensitivity = "variant";
    }

    ignorePunctuation = ParseOption(options, "ignorePunctuation");
    if (ignorePunctuation == "") {
        ignorePunctuation = "false";
    }

    numeric = ParseOption(options, "numeric");
    caseFirst = ParseOption(options, "caseFirst");
    collation = ParseOption(options, "collation");
}

Collator::Collator(std::vector<std::string> &localeTags, std::map<std::string, std::string> &options)
{
    ParseAllOptions(options);

    UErrorCode status = UErrorCode::U_ZERO_ERROR;
    if (localeTags.size() == 0) {
        localeTags.push_back(LocaleConfig::GetSystemLocale());
    }
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = icu::Locale::forLanguageTag(icu::StringPiece(curLocale), status);
        if (LocaleInfo::allValidLocales.count(locale.getLanguage()) > 0) {
            localeInfo = new LocaleInfo(curLocale, options);
            locale = localeInfo->GetLocale();
            localeStr = localeInfo->GetBaseName();
            bool createSuccess = InitCollator();
            if (!createSuccess) {
                continue;
            }
            break;
        }
    }
}

bool Collator::IsValidCollation(std::string &collation, UErrorCode &status)
{
    const char *currentCollation = uloc_toLegacyType("collation", collation.c_str());
    if (currentCollation != nullptr) {
        std::unique_ptr<icu::StringEnumeration> enumeration(
            icu::Collator::getKeywordValuesForLocale("collation", icu::Locale(locale.getBaseName()), false, status));
        int length;
        const char *validCollations = enumeration->next(&length, status);
        while (validCollations != nullptr) {
            if (strcmp(validCollations, currentCollation) == 0) {
                return true;
            }
            validCollations = enumeration->next(&length, status);
        }
    }
    return false;
}

void Collator::SetCollation(UErrorCode &status)
{
    if (collation != "") {
        if (IsValidCollation(collation, status)) {
            locale.setUnicodeKeywordValue("co", collation, status);
        } else {
            collation = "default";
            locale.setUnicodeKeywordValue("co", nullptr, status);
        }
    } else {
        collation = localeInfo->GetCollation();
        if (collation != "") {
            if (IsValidCollation(collation, status)) {
                locale.setUnicodeKeywordValue("co", collation, status);
            } else {
                locale.setUnicodeKeywordValue("co", nullptr, status);
                collation = "default";
            }
        } else {
            locale.setUnicodeKeywordValue("co", nullptr, status);
            collation = "default";
        }
    }
}

void Collator::SetUsage(UErrorCode &status)
{
    if (usage == "search") {
        collation = "default";
        locale.setUnicodeKeywordValue("co", nullptr, status);
    }
}

void Collator::SetNumeric(UErrorCode &status)
{
    if (numeric == "") {
        numeric = localeInfo->GetNumeric();
        if (numeric != "true" && numeric != "false") {
            numeric = "false";
        }
    }
    if (numeric == "true") {
        collatorPtr->setAttribute(UColAttribute::UCOL_NUMERIC_COLLATION,
            UColAttributeValue::UCOL_ON, status);
    } else {
        collatorPtr->setAttribute(UColAttribute::UCOL_NUMERIC_COLLATION,
            UColAttributeValue::UCOL_OFF, status);
    }
}

void Collator::SetCaseFirst(UErrorCode &status)
{
    if (caseFirst == "") {
        caseFirst = localeInfo->GetCaseFirst();
        if (caseFirst != "upper" && caseFirst != "lower" && caseFirst != "false") {
            caseFirst = "false";
        }
    }
    if (caseFirst == "upper") {
        collatorPtr->setAttribute(UColAttribute::UCOL_CASE_FIRST,
            UColAttributeValue::UCOL_UPPER_FIRST, status);
    } else if (caseFirst == "lower") {
        collatorPtr->setAttribute(UColAttribute::UCOL_CASE_FIRST,
            UColAttributeValue::UCOL_LOWER_FIRST, status);
    } else {
        collatorPtr->setAttribute(UColAttribute::UCOL_CASE_FIRST,
            UColAttributeValue::UCOL_OFF, status);
    }
}

void Collator::SetSensitivity(UErrorCode &status)
{
    if (sensitivity == "base") {
        collatorPtr->setStrength(icu::Collator::PRIMARY);
    } else if (sensitivity == "accent") {
        collatorPtr->setStrength(icu::Collator::SECONDARY);
    } else if (sensitivity == "case") {
        collatorPtr->setStrength(icu::Collator::PRIMARY);
        collatorPtr->setAttribute(UColAttribute::UCOL_CASE_LEVEL,
            UColAttributeValue::UCOL_ON, status);
    } else {
        collatorPtr->setStrength(icu::Collator::TERTIARY);
    }
}

void Collator::SetIgnorePunctuation(UErrorCode &status)
{
    if (ignorePunctuation == "true") {
        collatorPtr->setAttribute(UColAttribute::UCOL_ALTERNATE_HANDLING,
            UColAttributeValue::UCOL_SHIFTED, status);
    }
}

bool Collator::InitCollator()
{
    UErrorCode status = UErrorCode::U_ZERO_ERROR;
    SetCollation(status);
    SetUsage(status);
    collatorPtr = icu::Collator::createInstance(locale, status);
    SetNumeric(status);
    SetCaseFirst(status);
    SetSensitivity(status);
    SetIgnorePunctuation(status);

    if (collatorPtr == nullptr) {
        if (localeInfo != nullptr) {
            delete localeInfo;
            localeInfo = nullptr;
        }
        return false;
    }
    return true;
}

Collator::~Collator()
{
    if (localeInfo != nullptr) {
        delete localeInfo;
        localeInfo = nullptr;
    }

    if (collatorPtr != nullptr) {
        delete collatorPtr;
        collatorPtr = nullptr;
    }
}

int32_t Collator::Compare(const std::string &first, const std::string &second)
{
    icu::Collator::EComparisonResult result = collatorPtr->compare(icu::UnicodeString(first.data(), first.length()),
        icu::UnicodeString(second.data(), second.length()));
    if (result == icu::Collator::EComparisonResult::LESS) {
        return -1;
    } else if (result == icu::Collator::EComparisonResult::EQUAL) {
        return 0;
    } else {
        return 1;
    }
}

void Collator::ResolvedOptions(std::map<std::string, std::string> &options)
{
    options.insert(
        {
            { "locale", localeStr },
            { "usage", usage },
            { "sensitivity", sensitivity },
            { "ignorePunctuation", ignorePunctuation },
            { "numeric", numeric },
            { "caseFirst", caseFirst },
            { "collation", collation }
        }
    );
}
}
}
}