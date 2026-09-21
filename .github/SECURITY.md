# Security Policy

## Supported Versions

Fixes for security vulnerabilities are proactively applied to any applicable
branch/release series that is in the
[Next-Gen, Active, Maintenance, or Extended support category](https://TurboVNC.org/DeveloperInfo/Versioning).

## Reporting a Security Vulnerability

Suspected vulnerabilities can be reported in one of the following ways:

- [E-mail the project admin](https://TurboVNC.org/About/Contact).  You can
  optionally encrypt the e-mail using the provided public GPG key.

- If the issue affects only
  [Alpha/Evolving code](https://TurboVNC.org/DeveloperInfo/Versioning) or has
  otherwise not officially been released, then it is not (yet) a security
  vulnerability.  Such issues should be reported using a
  [GitHub bug report](https://github.com/TurboVNC/turbovnc/issues/new).

- If the issue affects only an
  [EOL](https://TurboVNC.org/DeveloperInfo/Versioning) branch/release series,
  then it is not a security vulnerability.  (Per above, fixes for security
  vulnerabilities are not proactively applied to EOL branches/release series.)
  Such issues can be reported using a
  [GitHub bug report](https://github.com/TurboVNC/turbovnc/issues/new), but the
  suggested remedy will likely be to upgrade to a supported release.

- [Beta and Post-Beta code](https://TurboVNC.org/DeveloperInfo/Versioning) is
  not expected to be free of bugs, so vulnerabilities that affect only that
  code (for example, vulnerabilities introduced by a new feature that is not
  present in a Stable release series) can optionally be reported using a
  [GitHub bug report](https://github.com/TurboVNC/turbovnc/issues/new).
