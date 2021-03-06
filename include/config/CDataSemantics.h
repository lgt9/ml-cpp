/*
 * Copyright Elasticsearch B.V. and/or licensed to Elasticsearch B.V. under one
 * or more contributor license agreements. Licensed under the Elastic License;
 * you may not use this file except in compliance with the Elastic License.
 */

#ifndef INCLUDED_ml_config_CDataSemantics_h
#define INCLUDED_ml_config_CDataSemantics_h

#include <maths/CBasicStatistics.h>
#include <maths/CBjkstUniqueValues.h>
#include <maths/COrdinal.h>

#include <config/ConfigTypes.h>
#include <config/ImportExport.h>

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <cstddef>

namespace ml {
namespace config {

//! \brief Determines the semantics of some data from examples.
//!
//! DESCRIPTION:\n
//! Currently, tries to identify binary categorical, categorical,
//! integer and real valued data.
//!
//! Inference of ordinal data verses categorical data can also be
//! achieved. See for example,
//! https://jmhldotorg.files.wordpress.com/2014/11/nipsworkshop2014learningsemantics4.pdf
//! Since we cannot currently use this information in the back end
//! we do not test for this.
//!
//! IMPLEMENTATION:\n
//! We assume that the data are passed by strings and that only
//! examples from a single data type, to be identified, are
//! supplied. If multiple data types need to be identified then
//! a different object should be used for each.
class CONFIG_EXPORT CDataSemantics {
public:
    using TOptionalUserDataType = boost::optional<config_t::EUserDataType>;

public:
    //! The proportion of values which must be numeric for the
    //! data to be a candidate metric.
    static const double NUMERIC_PROPORTION_FOR_METRIC_STRICT;

    //! The proportion of values which must be numeric for the
    //! data to be a candidate metric if there are only a small
    //! number of distinct non-numeric strings.
    static const double NUMERIC_PROPORTION_FOR_METRIC_WITH_SUSPECTED_MISSING_VALUES;

    //! The proportion of values which must be integer for the
    //! data to be a candidate integer.
    static const double INTEGER_PRORORTION_FOR_INTEGER;

public:
    explicit CDataSemantics(TOptionalUserDataType override = TOptionalUserDataType());

    //! Add an example from the data set.
    void add(const std::string& example);

    //! Compute the type of the data based on the examples added so far.
    void computeType();

    //! Get the last inferred data type set by computeType.
    config_t::EDataType type() const;

private:
    //! \brief Hashes an ordinal type.
    class CONFIG_EXPORT CHashOrdinal {
    public:
        std::size_t operator()(maths::COrdinal value) const {
            return value.hash();
        }
    };
    using TStrVec = std::vector<std::string>;
    using TOrdinalSizeUMap = boost::unordered_map<maths::COrdinal, std::size_t, CHashOrdinal>;
    using TMinAccumulator =
        maths::CBasicStatistics::COrderStatisticsStack<maths::COrdinal, 1>;
    using TMaxAccumulator =
        maths::CBasicStatistics::COrderStatisticsStack<maths::COrdinal, 1, std::greater<maths::COrdinal>>;

private:
    //! The maximum number of values we'll hold in the empirical
    //! distribution.
    static const std::size_t MAXIMUM_EMPIRICAL_DISTRIBUTION_SIZE;

private:
    //! Get the categorical type.
    config_t::EDataType categoricalType() const;

    //! Get the real type.
    config_t::EDataType realType() const;

    //! Get the integer type.
    config_t::EDataType integerType() const;

    //! Check if the field is numeric.
    bool isNumeric() const;

    //! Check if the field is integer.
    bool isInteger() const;

    //! Check how well the data is approximated by a Gaussian
    //! mixture model.
    bool GMMGoodFit() const;

    //! Add an integer value.
    template<typename INT>
    maths::COrdinal addInteger(INT value);

    //! Add a positive integer value.
    template<typename UINT>
    maths::COrdinal addPositiveInteger(UINT value);

    //! Add a real value.
    template<typename REAL>
    maths::COrdinal addReal(REAL value);

private:
    //! The last computed type.
    config_t::EDataType m_Type;

    //! Get a user specified override for the field type.
    TOptionalUserDataType m_Override;

    //! The total number of examples.
    double m_Count;

    //! True if the values are numeric.
    double m_NumericProportion;

    //! The proportion of values which are integer.
    double m_IntegerProportion;

    //! The smallest numerical value received.
    TMinAccumulator m_Smallest;

    //! The largest numerical value received.
    TMaxAccumulator m_Largest;

    //! The no more than three of the distinct values.
    TStrVec m_DistinctValues;

    //! Examples of non-numeric strings.
    TStrVec m_NonNumericValues;

    //! Set to true if there are too many distinct values to maintain
    //! the empirical distribution.
    bool m_EmpiricalDistributionOverflowed;

    //! The empirical distribution.
    TOrdinalSizeUMap m_EmpiricalDistribution;
};
}
}

#endif // INCLUDED_ml_config_CValueSemantics_h
