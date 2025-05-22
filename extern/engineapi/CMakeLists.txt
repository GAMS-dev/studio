cmake_minimum_required(VERSION 3.5)

project(gams_engine)

set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Gui)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(${PROJECT_NAME}
  OAIBad_input.h
  OAICleanable_job_result.h
  OAICleanable_job_result_page.h
  OAIEngine_license.h
  OAIFiles_not_found.h
  OAIForwarded_token_response.h
  OAIGeneric_key_value_pair.h
  OAIHypercube.h
  OAIHypercube_page.h
  OAIHypercube_summary.h
  OAIHypercube_token.h
  OAIIdentity_provider.h
  OAIIdentity_provider_ldap.h
  OAIIdentity_provider_oauth2.h
  OAIIdentity_provider_oauth2_scope.h
  OAIIdentity_provider_oauth2_with_secret.h
  OAIIdentity_provider_oidc.h
  OAIIdentity_provider_oidc_with_secret.h
  OAIInex.h
  OAIInvitation.h
  OAIInvitation_quota.h
  OAIInvitation_token.h
  OAIJob.h
  OAIJob_no_text_entry.h
  OAIJob_no_text_entry_page.h
  OAILicense.h
  OAILog_piece.h
  OAIMessage.h
  OAIMessage_and_token.h
  OAIMessage_with_webhook_id.h
  OAIModel_auth_token.h
  OAIModel_configuration.h
  OAIModel_default_instance.h
  OAIModel_default_user_instance.h
  OAIModel_hypercube_job.h
  OAIModel_hypercube_usage.h
  OAIModel_instance_info.h
  OAIModel_instance_info_full.h
  OAIModel_instance_pool_info.h
  OAIModel_job_labels.h
  OAIModel_job_usage.h
  OAIModel_pool_usage.h
  OAIModel_usage.h
  OAIModel_user.h
  OAIModel_userinstance_info.h
  OAIModel_userinstancepool_info.h
  OAIModel_version.h
  OAIModels.h
  OAINamespace.h
  OAINamespace_quota.h
  OAINamespace_with_permission.h
  OAINot_found.h
  OAIPassword_policy.h
  OAIPerm_and_username.h
  OAIQuota.h
  OAIQuota_exceeded.h
  OAIStatus_code_meaning.h
  OAIStream_entry.h
  OAISystem_wide_license.h
  OAIText_entries.h
  OAIText_entry.h
  OAITime_span.h
  OAITime_span_pool_worker.h
  OAIToken_forward_error.h
  OAIUser.h
  OAIUser_group_member.h
  OAIUser_groups.h
  OAIVapid_info.h
  OAIWebhook.h
  OAIWebhook_parameterized_event.h
  OAIAuthApi.h
  OAICleanupApi.h
  OAIDefaultApi.h
  OAIHypercubeApi.h
  OAIJobsApi.h
  OAILicensesApi.h
  OAINamespacesApi.h
  OAIUsageApi.h
  OAIUsersApi.h
  OAIHelpers.h
  OAIHttpRequest.h
  OAIObject.h
  OAIEnum.h
  OAIHttpFileElement.h
  OAIServerConfiguration.h
  OAIServerVariable.h
  OAIOauth.h
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
  OAIMessage_with_webhook_id.cpp
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
  OAIPassword_policy.cpp
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
  OAIVapid_info.cpp
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

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Gui

)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(
    FILES ${HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
    EXPORT ${PROJECT_NAME}Targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
