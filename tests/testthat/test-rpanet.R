test_that("Test rpanet with default preference functions", {
  # sample PA networks
  control <- rpactl.preference(ftype = "default",
                               sparams = c(1, 2, 1, 1.5, 1),
                               tparams = c(1, 1.5, 1, 2, 1),
                               params = c(1.5, 1)) +
    rpactl.scenario(alpha = 0.2, beta = 0.4, gamma = 0.2, xi = 0.1, rho = 0.1) +
    rpactl.edgeweight(distribution = rgamma, dparams = list(shape = 5, scale = 0.2))
  net1 <- rpanet(control = control, nstep = 2e5, directed = TRUE)
  net2 <- rpanet(control = control, nstep = 2e5, directed = FALSE)
  
  # check node preference
  n1 <- length(net1$outstrength)
  n2 <- length(net2$strength)
  sparams <- control$preference$sparams
  tparams <- control$preference$tparams
  params <- control$preference$params
  ret1.1 <- range(net1$spref - sparams[1] * net1$outstrength^sparams[2] - 
                    sparams[3] * net1$instrength^sparams[4] - sparams[5])
  ret1.2 <- range(net1$tpref - tparams[1] * net1$outstrength^tparams[2] - 
                    tparams[3] * net1$instrength^tparams[4] - tparams[5])
  ret2 <- range(net2$pref - net2$strength^params[1] - params[2])
  ret <- max(abs(c(ret1.1, ret1.2, ret2)))
  expect_lt(ret, 1e-5)
  
  # check node strength
  temp1 <- node_strength_cpp(net1$edgelist[, 1], 
                             net1$edgelist[, 2],
                             net1$edgeweight, 
                             max(net1$edgelist), 
                             TRUE)
  temp2 <- node_strength_cpp(net2$edgelist[, 1], 
                             net2$edgelist[, 2],
                             net2$edgeweight, 
                             max(net2$edgelist),
                             TRUE)
  ret1.1 <- range(net1$outstrength - temp1$outstrength)
  ret1.2 <- range(net1$instrength - temp1$instrength)
  ret2 <- range(net2$strength - temp2$outstrength - temp2$instrength)
  ret <- max(abs(c(ret1.1, ret1.2, ret2)))
  expect_lt(ret, 1e-5) 
})

# test_that("Test rpanet with customized preference functions", {
#   # sample PA networks
#   require(Rcpp)
#   require(RcppArmadillo)
#   control <- rpactl.preference(ftype = "customized",
#                                spref = "outs + pow(ins, 0.5) + 1",
#                                tpref = "pow(outs, 0.5) + ins + 1",
#                                pref = "pow(s, 1.5) + 1") +
#     rpactl.scenario(alpha = 0.2, beta = 0.4, gamma = 0.2, xi = 0.1, rho = 0.1) +
#     rpactl.edgeweight(distribution = rgamma, dparams = list(shape = 5, scale = 0.2))
#   net1 <- rpanet(control = control, nstep = 2e5, directed = TRUE)
#   net2 <- rpanet(control = control, nstep = 2e5, directed = FALSE)

#   # check node preference
#   n1 <- length(net1$outstrength)
#   n2 <- length(net2$strength)
#   ret1.1 <- range(net1$spref - sapply(1:n1, function(i) spref_func(net1$outstrength[i],
#                                                                    net1$instrength[i])))
#   ret1.2 <- range(net1$tpref - sapply(1:n1, function(i) tpref_func(net1$outstrength[i],
#                                                                    net1$instrength[i])))
#   ret2 <- range(net2$pref - sapply(1:n2, function(i) pref_func(net2$strength[i])))
#   ret <- max(abs(c(ret1.1, ret1.2, ret2)))
#   expect_lt(ret, 1e-5)

#   # check node strength
#   temp1 <- node_strength_cpp(net1$edgelist[, 1],
#                              net1$edgelist[, 2],
#                              net1$edgeweight,
#                              max(net1$edgelist),
#                              TRUE)
#   temp2 <- node_strength_cpp(net2$edgelist[, 1],
#                              net2$edgelist[, 2],
#                              net2$edgeweight,
#                              max(net2$edgelist),
#                              TRUE)
#   ret1.1 <- range(net1$outstrength - temp1$outstrength)
#   ret1.2 <- range(net1$instrength - temp1$instrength)
#   ret2 <- range(net2$strength - temp2$outstrength - temp2$instrength)
#   ret <- max(abs(c(ret1.1, ret1.2, ret2)))
#   expect_lt(ret, 1e-5)
# })