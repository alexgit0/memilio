#############################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors: Kathrin Rack, Wadim Koslow
#
# Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################
"""
@file cleanData.py

@brief Tool to delete data generated by some epidemiology.epidata packages

Data from following packages can be deleted:
- getRKIData.py
- getJHData.py
- getDIVIData.py
- getPopulationData
"""

import argparse
import os

from epidemiology.epidata import defaultDict as dd


def clean_data(all_data, rki, john_hopkins, population, hdf5, out_path):
    """! Function to delete data which is generated by epidemiology.epidata package

        Deletes all data or data from the different packages.

        @param all_data Defines if all possible data should be deleted. Default = False
        @param rki Defines if data generated by getRKIData.py should be deleted. Default = False
        @param john_hopkins Defines if data generated by getJHData.py should be deleted. Default = False
        @param population Defines if data generated by getDIVIData.py should be deleted. Default = False
        @param hdf5 Defines if files with ending json [Default] or hdf5 should be deleted.
        @param out_path Defines path where data should be deleted.
        """
	#TODO: clean new data: Vaccination, Testing

    ending = "json"
    if hdf5:
        ending = "h5"

    # TODO: get list from other packages which data they write.
    # Otherwise when changing anything this tool has to be changes as well

    if all_data:

        # TODO: make general dictionary with all countries used
        directories = ['Germany/', 'Spain/', 'France/',
                       'Italy/', 'US/', 'SouthKorea/', 'China/']

        # delete files in directory
        for cdir in directories:
            directory = os.path.join(out_path, cdir)

            try:
                files = os.listdir(directory)
            except FileNotFoundError:
                continue

            for item in files:
                if item.endswith(".json") or item.endswith(".h5"):
                    print("Deleting file ", os.path.join(directory, item))
                    os.remove(os.path.join(directory, item))

            # delete directories if empty
            try:
                os.rmdir(directory)
            except OSError:
                continue
            print("Deleting directory ", directory)

        # delete further jh files
        files = []
        try:
            files = os.listdir(out_path)
        except FileNotFoundError:
            pass

        for item in files:
            if item.endswith(".json") or item.endswith(".h5"):
                print("Deleting file ", os.path.join(out_path, item))
                os.remove(os.path.join(out_path, item))

    elif rki or john_hopkins or population:

        if rki:
            directory = os.path.join(out_path, 'Germany/')
            files = []
            try:
                files = os.listdir(directory)
            except FileNotFoundError:
                pass

            for item in files:
                if item.endswith(ending):
                    if "_rki" in item or "RKI" in item:
                        print("Deleting file ", os.path.join(directory, item))
                        os.remove(os.path.join(directory, item))

            # delete directory if empty
            try:
                os.rmdir(directory)
                print("Deleting directory ", directory)
            except OSError:
                pass

        if population:
            directory = os.path.join(out_path, 'Germany/')
            files = []
            try:
                files = os.listdir(directory)
            except FileNotFoundError:
                pass

            for item in files:
                if item.endswith(ending):
                    if "Popul" in item or "FullDataB" in item or "FullDataL" in item:
                        print("Deleting file ", os.path.join(directory, item))
                        os.remove(os.path.join(directory, item))

            # delete directory if empty
            try:
                os.rmdir(directory)
                print("Deleting directory ", directory)
            except OSError:
                pass

        if john_hopkins:
            # TODO: make general dictionary with all countries used
            directories = ['Germany/', 'Spain/', 'France/',
                           'Italy/', 'US/', 'SouthKorea/', 'China/']

            # delete files in directory
            for cdir in directories:
                directory = os.path.join(out_path, cdir)

                try:
                    files = os.listdir(directory)
                except FileNotFoundError:
                    continue

                for item in files:
                    if item.endswith(ending) and "_jh" in item:
                        print("Deleting file ", os.path.join(directory, item))
                        os.remove(os.path.join(directory, item))

                # delete directories
                try:
                    os.rmdir(directory)
                except OSError:
                    continue

                print("Deleting directory ", directory)

            # delete further jh files
            files = []
            try:
                files = os.listdir(out_path)
            except FileNotFoundError:
                pass

            for item in files:
                if item.endswith(ending):
                    if "_jh" in item or "JohnHopkins" in item:
                        print("Deleting file ", os.path.join(out_path, item))
                        os.remove(os.path.join(out_path, item))

    else:
        print("Please specify what should be deleted. See --help for details.")


def cli():
    """! Command line interface specific for clean data

    Via this interface it can defined which data should be deleted and where.
    Possibilities are

    - delete all
    - delete just rki, jh or population
    - choose file format: json or hdf5
    - define path to files
    """

    out_path_default = dd.defaultDict['out_folder']

    parser = argparse.ArgumentParser()

    parser.add_argument('-a', '--all-data',
                        help='Deletes all data and folders which could be possibly written by the epidata package.',
                        action='store_true')
    parser.add_argument('-r', '--rki', help='Deletes just rki data.',
                        action='store_true')
    parser.add_argument('-j', '--john-hopkins', help='Deletes just data from John Hopkins university.',
                        action='store_true')
    parser.add_argument('-p', '--population', help='Deletes just population data.',
                        action='store_true')

    parser.add_argument('-h5', '--hdf5', help='Deletes just hdf5 files.',
                        action='store_true')
    parser.add_argument('-o', '--out_path', type=str,
                        default=out_path_default, help='Defines folder for output.')

    args = parser.parse_args()

    return [args.all_data, args.rki, args.john_hopkins, args.population, args.hdf5, args.out_path]


def main():
    """! Main program entry."""

    [all_data, rki, john_hopkins, population, hdf5, out_path] = cli()

    clean_data(all_data, rki, john_hopkins, population, hdf5, out_path)


if __name__ == "__main__":
    main()
