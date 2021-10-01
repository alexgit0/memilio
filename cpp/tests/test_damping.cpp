/* 
* Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
*
* Authors: Daniel Abele, Martin J. Kuehn
*
* Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
*
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
#include "memilio/epidemiology/damping.h"
#include "matchers.h"
#include <gtest/gtest.h>

TEST(TestDampings, initZero)
{
    epi::Dampings<epi::Damping<epi::RectMatrixShape>> dampings(3, 2);
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-1e5)), print_wrap(Eigen::MatrixXd::Zero(3, 2)));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(0)), print_wrap(Eigen::MatrixXd::Zero(3, 2)));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(1e-32)), print_wrap(Eigen::MatrixXd::Zero(3, 2)));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(1e5)), print_wrap(Eigen::MatrixXd::Zero(3, 2)));
}

TEST(TestDampings, dampingsOnDifferentLevels)
{
    epi::Dampings<epi::Damping<epi::RectMatrixShape>> dampings(2, 2);
    auto D1 = 0.25;
    auto D2 = (Eigen::MatrixXd(2, 2) << 0.25, 0.5, 0.75, 1).finished();
    dampings.add(D1, epi::DampingLevel(7), epi::DampingType(3), epi::SimulationTime(0.5));
    dampings.add(D2, epi::DampingLevel(13), epi::DampingType(3), epi::SimulationTime(2.0));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-1e5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-0.5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(0.5 + 1e-32)), MatrixNear(Eigen::MatrixXd::Constant(2, 2, D1)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(1e5)), MatrixNear((D1 + D2.array() - D1 * D2.array()).matrix()));
}

TEST(TestDampings, dampingsOnSameLevel)
{
    epi::Dampings<epi::Damping<epi::SquareMatrixShape>> dampings(2);
    auto D1 = 0.25;
    auto D2 = (Eigen::MatrixXd(2, 2) << 0.0, 0.25, 0.5, 0.75).finished();
    dampings.add(D1, epi::DampingLevel(-2), epi::DampingType(0), epi::SimulationTime(0.5));
    dampings.add(D2, epi::DampingLevel(-2), epi::DampingType(1), epi::SimulationTime(2.0));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-1e5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-0.5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(0.5 + 1e-32)), MatrixNear(Eigen::MatrixXd::Constant(2, 2, D1)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(1e5)), MatrixNear((D1 + D2.array()).matrix()));
}

TEST(TestDampings, dampingsAtTheSameTime)
{
    epi::Dampings<epi::Damping<epi::SquareMatrixShape>> dampings(2);
    auto D1 = 0.25;
    auto D2 = (Eigen::MatrixXd(2, 2) << 0.0, 0.25, 0.5, 0.75).finished();
    dampings.add(D1, epi::DampingLevel(-2), epi::DampingType(0), epi::SimulationTime(0.5));
    dampings.add(D2, epi::DampingLevel(-2), epi::DampingType(1), epi::SimulationTime(0.5));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(-0.5)), MatrixNear(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(0.5 + 1e-5)), MatrixNear((D1 + D2.array()).matrix()));
}

TEST(TestDampings, dampingOfSameType)
{
    epi::Dampings<epi::Damping<epi::SquareMatrixShape>> dampings(2);
    auto D1 = 0.25;
    auto D2 = (Eigen::MatrixXd(2, 2) << 0.0, 0.25, 0.5, 0.75).finished();
    dampings.add(D1, epi::DampingLevel(123), epi::DampingType(5), epi::SimulationTime(0.5));
    dampings.add(D2, epi::DampingLevel(123), epi::DampingType(5), epi::SimulationTime(2.0));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-1e5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_EQ(print_wrap(dampings.get_matrix_at(-0.5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(0.5 + 1e-32)), MatrixNear(Eigen::MatrixXd::Constant(2, 2, D1)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(1e5)), MatrixNear(D2));
}

TEST(TestDampings, dampingsCombined)
{
    epi::Dampings<epi::Damping<epi::SquareMatrixShape>> dampings(2);
    auto D1 = 0.25;
    auto D2 = (Eigen::MatrixXd(2, 2) << 0.1, 0.1, 0.1, 0.1).finished();
    auto D3 = (Eigen::MatrixXd(2, 2) << 0.0, 0.25, 0.5, 0.75).finished();
    auto D4 = 0.5;
    //add dampings out of order to check sorting
    dampings.add(D2, epi::DampingLevel(7), epi::DampingType(2), epi::SimulationTime(0.0));
    dampings.add(D1, epi::DampingLevel(123), epi::DampingType(5), epi::SimulationTime(-2.0));
    dampings.add(D4, epi::DampingLevel(123), epi::DampingType(5), epi::SimulationTime(3.0));
    dampings.add(D3, epi::DampingLevel(7), epi::DampingType(3), epi::SimulationTime(1.5));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(-1e5)), print_wrap(Eigen::MatrixXd::Zero(2, 2)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(-1.0)), MatrixNear(Eigen::MatrixXd::Constant(2, 2, D1)));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(0.2)), MatrixNear((D1 + D2.array() - D1 * D2.array()).matrix()));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(2.0)),
                MatrixNear((D1 + D2.array() + D3.array() - D1 * (D2 + D3).array()).matrix()));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(1e45)),
                MatrixNear((D4 + D2.array() + D3.array() - D4 * (D2 + D3).array()).matrix()));
}

TEST(TestDampings, smoothTransitions)
{
    epi::Dampings<epi::Damping<epi::ColumnVectorShape>> dampings(2);
    auto D1 = 0.25;
    auto D2 = (Eigen::VectorXd(2) << 0.1, 0.1).finished();
    dampings.add(D1, epi::DampingLevel(123), epi::DampingType(5), epi::SimulationTime(-2.0));
    dampings.add(D2, epi::DampingLevel(1), epi::DampingType(10), epi::SimulationTime(1.5));

    EXPECT_THAT(print_wrap(dampings.get_matrix_at(-2.5)),
                MatrixNear((dampings.get_matrix_at(-3.) + dampings.get_matrix_at(-2.)) / 2));
    EXPECT_THAT(print_wrap(dampings.get_matrix_at(1.0)),
                MatrixNear((dampings.get_matrix_at(0.5) + dampings.get_matrix_at(1.5)) / 2));
}