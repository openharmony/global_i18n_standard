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
#ifndef OHOS_GLOBAL_I18N_NUMBER_FORMAT_H
#define OHOS_GLOBAL_I18N_NUMBER_FORMAT_H

#include "unicode/locid.h"
#include "unicode/numfmt.h"
#include "unicode/unum.h"
#include "unicode/localebuilder.h"
#include "unicode/numsys.h"
#include "number_utils.h"
#include "number_utypes.h"
#include "locale_info.h"
#include <map>
#include <set>
#include <vector>

namespace OHOS {
namespace Global {
namespace I18n {
class NumberFormat {
public:
    NumberFormat(const std::vector<std::string> &localeTag, std::map<std::string, std::string> &configs);
    virtual ~NumberFormat();
    std::string Format(double number);
    void GetResolvedOptions(std::map<std::string, std::string> &map);
    std::string GetCurrency();
    std::string GetCurrencySign();
    std::string GetStyle();
    std::string GetNumberingSystem();
    std::string GetUseGrouping();
    std::string GetMinimumIntegerDigits();
    std::string GetMinimumFractionDigits();
    std::string GetMaximumFractionDigits();

private:
    icu::Locale locale;
    std::string currency;
    std::string currencySign;
    std::string styleString;
    std::string numberingSystem;
    std::string useGrouping;
    std::string minimumIntegerDigits;
    std::string minimumFractionDigits;
    std::string maximumFractionDigits;
    std::string localeBaseName;
    LocaleInfo *localeInfo;
    icu::NumberFormat *numberFormat;
    UNumberFormatStyle style = UNumberFormatStyle::UNUM_DECIMAL;
    static std::map<std::string, UNumberFormatStyle> formatStyle;
    std::set<std::string> allValidLocales;
    void ParseConfigs(std::map<std::string, std::string> &configs);
    void GetValidLocales();
    void InitProperties();
    static bool icuInitialized;
    static bool Init();
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif