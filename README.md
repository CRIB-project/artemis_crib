# artemis-crib

[![Build and Deploy Docs](https://github.com/CRIB-project/artemis_crib/actions/workflows/deploy.yml/badge.svg)](https://github.com/CRIB-project/artemis_crib/actions/workflows/deploy.yml)

Artemis work directory for online+offline analysis of CRIB experiment

## License

Copyright 2013-2014 Shinsuke Ota <ota@cns.s.u-tokyo.ac.jp>, <ota@rcnp.osaka-u.ac.jp> and ARTEMIS develop team

## Quick Start

1. Install and setup ROOT
2. Install [Artemis](https://github.com/artemis-dev/artemis/tree/develop) from `develop` branch
3. (Option) Install [TSrim](https://www.cns.s.u-tokyo.ac.jp/gitlab/hayakawa/tsrim)
4. Setup `art_analysis` (working directory)

   ```shell
   curl -fsSL --proto '=https' --tlsv1.2 https://crib-project.github.io/artemis_crib/scripts/init.sh | sh
   ```

5. Set environment variables

   Example:

   ```bash
   export EXP_NAME="expname" # your experiment (for artlogin command)
   ```

6. Make new experiment repository

   ```shell
   artnew
   ```

7. Make user working directory

   ```shell
   artlogin <username>
   ```

8. Start analysis

   ```shell
   a
   ```

## For Details

<p><a href="https://crib-project.github.io/artemis_crib/index.html" target="_blank">CRIB artemis manual</a></p>

## Reference

- [artemis source](https://github.com/artemis-dev/artemis/tree/develop)
- [CRIB homepage](https://www.cns.s.u-tokyo.ac.jp/crib/crib-new/home-jp/)
