#############################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors: Rene Schmieding
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
import memilio
import sys
import time
import threading
from os import get_terminal_size

class ProgressIndicator:
    """! Print an animation to show that something is happening.
    
    Animations are rendered in a new thread, which is set up as deamon so that it stops on main thread exit.
    The members Dots, Spinner and Percentage provide some default animations.
    Start the animation by either using the `start`/`stop` functions or a 'with as' block, e.g
    ```
    with ProgressIndicator.Spinner():
        do_something()
    ```
    or
    ```
    with ProgressIndicator.Percentage() as indicator:
        for i in range(n) :
            do_something()
            indicator.set_progress((i+1)/n)
    ```
    """
    def __init__(self, animator, delay):
        """! Create an ProgressIndicator.
        
        @param animator generator expression. `next(animator)` must be a string
        @param delay positive real number. Sets delay in seconds between drawing animation frames.
        """
        assert(delay > 0)
        self._animator = animator # generator, in particular next(animator) must return a string
        self._delay = delay # some double
        self._thread = None
        self._enabled = False

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exception, value, trace):
        self.stop()

    def _render(self):
        """! Regularly update the animation. Do not call manually!"""
        while self._enabled:
            self.show()
            # wait before writing next frame 
            time.sleep(self._delay)

    def show(self):
        """! Print and advance the animation. """
        # write animation and proceed to next frame
        sys.stdout.write(" {}\r".format(next(self._animator)))
        sys.stdout.flush()

    def start(self):
        """! Start the animation in a new thread. Must call stop afterwards. """
        if not self._enabled:
            self._enabled = True
            # start new threat to render the animator in the background
            self._thread = threading.Thread(target=self._render)
            self._thread.setDaemon(True) # stop this thread on main thread exit
            self._thread.start()

    def stop(self):
        """! Stop the animation and join the thread. """
        if self._enabled:
            self._enabled = False
            if self._thread and self._thread.is_alive():
                self._thread.join()
            sys.stdout.write("\033[K") # clear line
    
    @classmethod
    def Spinner(class_object, delay=0.1, message=""):
        """! initializes a ProgressIndicator with a rotating line animation.
        
        This method spawns a new thread to print the animation.
        Start the animation by either using the `start`/`stop` functions or a 'with' block.

        @param delay    [Default: 0.1] positive real number. Sets delay in seconds between drawing animation frames.
        @param message  [Default: ""]  string. Text shown before the indicator (consider appeding a space as separator).
        """
        # prevent spamming output with messages longer than a single line
        if get_terminal_size().columns < len(message) + 2:
            print(message)
            message = ""

        def spin():
            while True: # loop animation
                for s in "|/-\\": # iterate animation frames
                    yield "{}{}".format(message, s) # return single frame
        return class_object(spin(), delay)

    @classmethod
    def Dots(class_object, delay=1, message="", num_dots=3, dot=".", blank=" "):
        """! initializes ProgressIndicator with a 'dot, dot, dot' animation.

        This method spawns a new thread to print the animation.
        Start the animation by either using the `start`/`stop` functions or a 'with' block.

        @param delay    [Default: 1]   positive real number. Sets delay in seconds between drawing animation frames.
        @param message  [Default: ""]  string. Text shown before the indicator (consider appeding a space as separator).
        @param num_dots [Default: 3]   positive integer. Determines maximum number of dots drawn.
        @param dot      [Default: "."] string. Drawn sequentially up to num_dots times. 
        @param blank    [Default: " "] string. Placeholder for yet to be drawn dots. Must have same length as dot.
        """
        assert(len(dot) == len(blank))
        assert(num_dots > 0)
        # prevent spamming output with messages longer than a single line
        if get_terminal_size().columns < num_dots + len(message) + 1:
            print(message)
            message = ""

        def dots():
            while True: # loop animation
                for n in range(1, num_dots+1): # iterate animation frames
                    yield "{}{}{}".format(message, dot*n, blank*(num_dots-n)) # return single frame
        return class_object(dots(), delay)

    class Percentage:
        def __init__(self, delay=1, message="", percentage=0, use_bar=False, use_delayed_output=True, keep_output=True):
            """! initializes ProgressIndicator showing a percantage, updated by 'set_progress'.

            By default, this method spawns a new thread to print the animation.
            If use_delayed_output is set to False, the delay is ignored, and no new thread is spawned. The output is
            then updated in the main thread, whenever 'set_progress' is called.
            Start the animation by either using the `start`/`stop` functions or a 'with as' block.
            The percentage can be updated using the set_progress member function.

            @param delay              [Default: 1]     positive real number. Sets delay in seconds between drawing animation frames.
            @param message            [Default: ""]    string. Text shown before the indicator (consider appeding a space as separator).
            @param percentage         [Default: 0]     real number in [0, 1]. Initial percentage for animation.
            @param use_bar            [Default: False] bool. If True, adds a bar plotting the current progress.
            @param use_delayed_output [Default: True]  bool. If False, delay is ignored and the animation is printed in the main thread.
            @param keep_output        [Default: True]  bool. Specifies whether the last animation frame should be kept in a new line.
            """
            self._use_thread = use_delayed_output
            self._keep_output = keep_output
            # use lists to pass variables by reference
            self._progress = [percentage]
            width = get_terminal_size().columns
            if use_bar:
                # prevent spamming output with messages longer than a single line
                if width < len(message) + 12:
                    print(message)
                    message = ""
                self._message = self._bar(self._progress, message, width)
            else:
                # prevent spamming output with messages longer than a single line
                if width < len(message) + 8:
                    print(message)
                    message = ""
                self._message = [message]
            self._indicator = ProgressIndicator(self._perc(self._progress, self._message), delay)

        def __enter__(self):
            self.start()
            return self

        def __exit__(self, exception, value, trace):
            self.stop()    

        def start(self):
            """! Start the animation. Must call stop afterwards. """
            if self._use_thread:
                self._indicator.start()

        def stop(self):
            """! Stops the animation. """
            if self._use_thread:
                self._indicator.show() # print manually to catch current progress
            if self._keep_output:
                sys.stdout.write("\n") # newline to keep output
            if self._use_thread:
                self._indicator.stop()
            else:
                sys.stdout.write("\033[K") # clear line


        def set_progress(self, percentage):
            """! Updates the percentage shown by the indicator.
            
            @param percentage real number. Must be in the interval [0, 1].
            """
            self._progress[0] = percentage
            if not self._indicator._enabled:
                self._indicator.show()

        @staticmethod
        # method to create animator function. uses lists to get variables by reference
        def _perc(p, message):
            while True:
                yield "{}{:6.2f}%".format(message[0], 100*p[0])

        # uses [] to return a progress bar string if use_bar is enabled
        class _bar:
            def __init__(self, percentage, message, width):
                self.p = percentage
                self.m = message
                self.w = width - len(message) - 11 # collected offset by percentage/extra characters
            def __getitem__(self, _): # ignore key
                n = int(self.w * self.p[0])
                return self.m + "[" + "#" * n + " " * (self.w - n) + "] "

if __name__ == "__main__":
    print("This is only a usage example, and does not actually do anything.")
    # start/stop
    p = ProgressIndicator.Dots(message="waiting", delay=0.5)
    p.start()
    time.sleep(1.6)
    p.stop()
    # with as
    with ProgressIndicator.Percentage(message="download 1 ", use_bar=True, delay=0.4) as p:
        for i in range(13):
            time.sleep(0.1467)
            p.set_progress((i+1)/13)
    with ProgressIndicator.Percentage(message="download 2 ", use_delayed_output=False, keep_output=False) as p:
        for i in range(97):
            time.sleep(0.0367)
            p.set_progress((i+1)/97)
    # with
    with ProgressIndicator.Spinner(message="finish "):
        time.sleep(2)