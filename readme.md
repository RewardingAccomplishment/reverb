<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>


<!-- ABOUT THE PROJECT -->
## About The Project

[![Reverb][product-screenshot]](https://avatars.githubusercontent.com/u/38396659?v=4)

Implementation of reverberation algorithm with test and simulation and run on DevBoard: STM32F769I.
In reverb algorithm was implemented a Schroeder Reverberator called JCRev with coefficients from [Documentation](https://ccrma.stanford.edu/~jos/Reverb/Reverb.pdf)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

Instructions how to setting up project locally.
To get a local copy up and running follow these simple example steps.

<pre>repo tree:
├── CMakeLists.txt
├── inc
│   └── reverb.h
├── LICENSE
├── readme.md
├── simulation
│   ├── effects
│   │   └── reverb.py
│   ├── preamble10.wav
│   └── test.py
└── src
    └── reverb.c

4 directories, 8 files
</pre>

### Prerequisites

<!-- This is an example of how to list things you need to use the software and how to install them.
* npm
  ```sh
  npm install npm@latest -g
  ``` -->

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/RewardingAccomplishment/reverb.git
   ```
2. Goto reberb directory and create build directory
   ```sh
   cd reverb
   mkdir build
   ```
3. Build the libraray
   ```sh
   cmake && make
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

Library implements reverb effect. And that effect could be check:
1. Goto reverb/simulation directory
    ```sh
   cd <path to repo>/reverb/simulation
   ```
2. Run the python test script as an argument you could pass the wav source file (for example there is a file preamble10.wav):
   ```sh
   ./test.py --source preamble10.wav
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [x] Python simulation script
- [x] Implementation of C lib
- [ ] Tests with DevBoard: STM32F769I
    - [x] input from mems microphone to line out
    - [x] applying the reverb effect
    - [ ] user interface on lcd screen

TODO:
- [ ] user interface:
    - [ ] bar for sound volume
    - [ ] bar for reverb filter parameter (change the value for each gain and number of smaples)
    - [ ] on/off record functionality
- [ ] makefile (now makefile is generated from ac6studio internal builder)
- [ ] noise reduction
- [ ] bug fixing


See the [open issues](https://github.com/RewardingAccomplishment/reverb/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the GNU Public License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## Contact

Marcin Sosnowski - marcin.sosnow@gmail.com

Project Link: [Reverb](https://github.com/RewardingAccomplishment/reverb)

<p align="right">(<a href="#readme-top">back to top</a>)</p>