# artemis-crib

Artemis work directory for online+offline analysis of CRIB experiment

# License

Copyright 2013-2014 Shinsuke Ota <ota@cns.s.u-tokyo.ac.jp>, <ota@rcnp.osaka-u.ac.jp> and ARTEMIS develop team

# Reference

- [artemis source](https://github.com/artemis-dev/artemis/tree/develop)
- [CRIB homepage](https://www.cns.s.u-tokyo.ac.jp/crib/crib-new/home-jp/)

# Installation

1. install and setup ROOT
2. install [artemis](https://github.com/artemis-dev/artemis/tree/develop) from develop branch
3. (Option) install [SRIMlib](https://github.com/okawak/SRIMlib) (LEGACY CODE) or [TSrim](https://www.cns.s.u-tokyo.ac.jp/gitlab/hayakawa/tsrim) (recommended)
4. setup art_analysis and this work directory

```shell
$ curl --proto '=https' --tlsv1.2 -sSf https://okawak.github.io/artemis_crib/bin/init.sh | sh
# setting your environment
$ vi ${HOME}/.bashrc
$ vi ${ZDOTDIR:-${HOME}}/.zshrc
```
```bash
export EXP_NAME="expname" # your experiment (for artlogin command)
export EXP_NAME_OLD="expname" # this is option (for artlogin2 command)
export PATH="${HOME}/art_analysis/bin:${PATH}"
source ${HOME}/art_analysis/bin/art_setting -q
```
```shell
$ source ${HOME}/.bashrc
$ source ${ZDOTDIR:-${HOME}}/.zshrc
$ artnew
# artnew is installed in ${HOME}/art_analysis/bin
```

For the detail, please check the github pages.
<p><a href="https://okawak.github.io/artemis_crib/" target="_blank">CRIB artemis manual</a></p>
