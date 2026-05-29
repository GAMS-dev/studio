**Release Steps**

- [ ] ensure `#.##.x-release` exists
- [ ] proofread `CHANGELOG` - remove `*`
- [ ] update `version` 
  - ++studio
  - ensure GAMS minor version is GAMS xx.**1**
- [ ] Test the last build
  - [ ] WEI
  - [ ] LEG
  - [ ] DEG
- [ ] create TAG for `release` branch
- [ ] merge release into develop
- [ ] merge release into master
- [ ] email to staff about the release
- [ ] proceed with products release MR

/assign me
/request_review me
/draft
