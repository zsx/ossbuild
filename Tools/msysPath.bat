@set MY_VAR=%~dp1
@set MY_VAR=%MY_VAR::=%
@set MY_VAR=%MY_VAR:\=/%
@set MY_VAR=/%MY_VAR%
@REM set MY_VAR=%MY_VAR: =\ %

@echo %MY_VAR%