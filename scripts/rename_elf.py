Import("env")

env.Replace(PROGNAME="fhatos_%s" % env.GetProjectConfig().get("fhatos","version"))