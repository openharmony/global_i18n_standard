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

declare namespace intl {
/**
 * Provides APIs for obtaining locale information.
 *
 * @since 6
 */
export class Locale {
    /**
     * A constructor used to create a Locale object.
     *
     * @param locale Indicates a character string containing the locale information, including
     *               the language and optionally the script and region.
     * @since 6
     */
   constructor(locale?: string);

    /**
     * Indicates the language of the locale.
     *
     * @since 6
     */
    language: string

    /**
     * Indicates the script of the locale.
     *
     * @since 6
     */
    script: string

    /**
     * Indicates the region of the locale.
     *
     * @since 6
     */
    region: string

    /**
     * Indicates the basic locale information, which is returned as a substring of
     * a complete locale string.
     *
     * @since 6
     */
    baseName: string

    /**
     * Indicates whether it is case first.
     */
    caseFirst: boolean

    /**
     * Indicates the calendar.
     */
    calendar: string

    /**
     * Indicates the collation.
     */
    collation: string

    /**
     * Indicates the hour cycle.
     */
    hourCycle:  string

    /**
     * Indicates the numbering system.
     */
    numberingSystem: string

    /**
     * Indicates whether it is numeric.
     */
    numeric: boolean

    /**
     * Convert the locale information to string.
     *
     * @return Returns locale information in string form.
     */
    toString(): string;
    maximize(): Locale;
    minimize(): Locale;
}

/**
 * Provides the options of date time format.
 */
export interface DateTimeOptions {
    /**
     * Indicates the locale.
     */
    locale: string

    /**
     * Indicates the date style.
     */
    dateStyle: string

    /**
     * Indicates the time style.
     */
    timeStyle: string

    /**
     * Indicates the hour cycle.
     */
    hourCycle: string

    /**
     * Indicates the timezone.
     */
    timeZone: string

    /**
     * Indicates the numbering system.
     */
    numberingSystem: string

    /**
     * Indicates whether is 12 hour or not.
     */
    hour12: boolean

    /**
     * Indicates the weekday style.
     */
    weekday: string

    /**
     * Indicates the era style.
     */
    era: string

    /**
     * Indicates the year style.
     */
    year: string

    /**
     * Indicates the month style.
     */
    month: string

    /**
     * Indicates the day style.
     */
    day: string

    /**
     * Indicates the hour style.
     */
    hour: string

    /**
     * Indicates the minute sty'.
     */
    minute: string

    /**
     * Indicates the second style.
     */
    second: string

    /**
     * Indicates the timezone name.
     */
    timeZoneName: string

    /**
     * Indicates the day period format.
     */
    dayPeriod: string

    /**
     * Indicates the locale matching algorithm.
     */
    localeMatcher: string

    /**
     * Indicates the format matching algorithm.
     */
    foramtMatcher: string
}

/**
 * Provides the API for formatting date strings.
 *
 * @since 6
 */
export class DateTimeFormat {
    /**
     * A constructor used to create a DateTimeFormat object.
     *
     * @param locale Indicates a character string containing the locale information, including
     *               the language and optionally the script and region, for the DateTimeFormat object.
     * @param options Indicates the options used to format the date.
     * @since 6
     */
    constructor(locale: string, options?: DateTimeOptions);

    /**
     * A constructor used to create a DateTimeFormat object.
     *
     * @param locale Indicates an array of character string containing the locale information, including
     *               the language and optionally the script and region, for the DateTimeFormat object.
     * @param options Indicates the options used to format the date.
     * @since 6
     */
    constructor(locale: string[], options?: DateTimeOptions);

    /**
     * Obtains the formatted date strings.
     *
     * @param date Indicates the Date object to be formatted.
     * @return Returns a date string formatted based on the specified locale.
     * @since 6
     */
    format(date: Date): string;

    /**
     * Obtains the formatted date strings of a date range.
     *
     * @param startDate Indicates the start date of the date range.
     * @param endDate Indicates the end date of the date range.
     * @return Returns a date string formatted based on the specified locale.
     * @since 6
     */
    formatRange(startDate: Date, endDate: Date): string;

    /**
     * Obtains the options of the DateTimeFormat object.
     *
     * @return Returns the options of the DateTimeFormat object.
     * @since 6
     */
    getResolvedOptions(): DateTimeOptions;
}

/**
 * Provides the options of number format.
 */
export interface NumberOptions {
    /**
     * Indicates the locale.
     */
    locale: string

    /**
     * Indicates the currency.
     */
    currency: string

    /**
     * Indicates the currency sign.
     */
    currencySign: string

    /**
     * Indicates the currency display format.
     */
    currencyDisplay: string

    /**
     * Indicates the unit.
     */
    unit: string

    /**
     * Indicates the unit display format.
     */
    unitDisplay: string

    /**
     * Indicates the sign display format.
     */
    signDisplay: string

    /**
     * Indicates the compact display format.
     */
    compactDisplay: string

    /**
     * Indicates the notation.
     */
    notation: string

    /**
     * Indicates the locale matching algorithm.
     */
    localeMatcher: string

    /**
     * Indicates the style.
     */
    style: string

    /**
     * Indicates the numbering system.
     */
    numberingSystem: string

    /**
     * Indicates whether using grouping or not.
     */
    useGrouping: boolean

    /**
     * Indicates the minimum integer digits.
     */
    minimumIntegerDigits: number

    /**
     * Indicates the minimum fraction digits.
     */
    minimumFractionDigits: number

    /**
     * Indicates the maximum fraction digits.
     */
    maximumFractionDigits: number

    /**
     * Indicates the minimum siginificant digits.
     */
    minimumSiginificantDigits: number

    /**
     * Indicates the maximum siginificant digits.
     */
    maximumSiginificantDigits: number
}

/**
 * Provides the API for formatting numebr strings.
 */
export class NumberFormat {
    constructor(locale: string, options?: NumberOptions);
    constructor(locale: string[], options?: NumberOptions);
    format(number: number): string;
    resolvedOptions(): NumberOptions;
}
}
export default intl;