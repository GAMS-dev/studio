cmake_minimum_required(VERSION 3.2)

project(client)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-unused-variable")
endif ()

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

add_library(${PROJECT_NAME}
    OAIBad_input.cpp
    OAICleanable_job_result.cpp
    OAICleanable_job_result_page.cpp
    OAIEngine_license.cpp
    OAIFiles_not_found.cpp
    OAIForwarded_token_response.cpp
    OAIGeneric_key_value_pair.cpp
    OAIHypercube.cpp
    OAIHypercube_page.cpp
    OAIHypercube_summary.cpp
    OAIHypercube_token.cpp
    OAIIdentity_provider.cpp
    OAIIdentity_provider_ldap.cpp
    OAIIdentity_provider_oauth2.cpp
    OAIIdentity_provider_oauth2_scope.cpp
    OAIIdentity_provider_oauth2_with_secret.cpp
    OAIIdentity_provider_oidc.cpp
    OAIIdentity_provider_oidc_with_secret.cpp
    OAIInex.cpp
    OAIInvitation.cpp
    OAIInvitation_quota.cpp
    OAIInvitation_token.cpp
    OAIJob.cpp
    OAIJob_no_text_entry.cpp
    OAIJob_no_text_entry_page.cpp
    OAILicense.cpp
    OAILog_piece.cpp
    OAIMessage.cpp
    OAIMessage_and_token.cpp
    OAIModel_auth_token.cpp
    OAIModel_configuration.cpp
    OAIModel_default_instance.cpp
    OAIModel_default_user_instance.cpp
    OAIModel_hypercube_job.cpp
    OAIModel_hypercube_usage.cpp
    OAIModel_instance_info.cpp
    OAIModel_instance_info_full.cpp
    OAIModel_instance_pool_info.cpp
    OAIModel_job_labels.cpp
    OAIModel_job_usage.cpp
    OAIModel_pool_usage.cpp
    OAIModel_usage.cpp
    OAIModel_user.cpp
    OAIModel_userinstance_info.cpp
    OAIModel_userinstancepool_info.cpp
    OAIModel_version.cpp
    OAIModels.cpp
    OAINamespace.cpp
    OAINamespace_quota.cpp
    OAINamespace_with_permission.cpp
    OAINot_found.cpp
    OAIPerm_and_username.cpp
    OAIQuota.cpp
    OAIQuota_exceeded.cpp
    OAIStatus_code_meaning.cpp
    OAIStream_entry.cpp
    OAISystem_wide_license.cpp
    OAIText_entries.cpp
    OAIText_entry.cpp
    OAITime_span.cpp
    OAITime_span_pool_worker.cpp
    OAIToken_forward_error.cpp
    OAIUser.cpp
    OAIUser_group_member.cpp
    OAIUser_groups.cpp
    OAIWebhook.cpp
    OAIWebhook_parameterized_event.cpp
    OAIAuthApi.cpp
    OAICleanupApi.cpp
    OAIDefaultApi.cpp
    OAIHypercubeApi.cpp
    OAIJobsApi.cpp
    OAILicensesApi.cpp
    OAINamespacesApi.cpp
    OAIUsageApi.cpp
    OAIUsersApi.cpp
    OAIHelpers.cpp
    OAIHttpRequest.cpp
    OAIHttpFileElement.cpp
    OAIOauth.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Network)
if(NOT APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE ssl crypto)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
