TEMPLATE = subdirs
symbian {
	system(call makesis -s tdriver_package_stub.pkg tdriver_package_stub.sis)
    # iby files
    BLD_INF_RULES.prj_exports += "tdriver_package.iby CORE_MW_LAYER_IBY_EXPORT_PATH(tdriver_package.iby)"
    BLD_INF_RULES.prj_exports += "tdriver_package_stub.sis  /epoc32/data/z/system/install/tdriver_package_stub.sis"
}
