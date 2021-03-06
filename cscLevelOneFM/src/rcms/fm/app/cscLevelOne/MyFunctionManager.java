package rcms.fm.app.cscLevelOne;

import java.util.List;
import java.util.ArrayList;

import rcms.fm.app.cscLevelOne.util.MyUtil;
import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserFunctionManager;
import rcms.fm.resource.QualifiedGroup;
import rcms.fm.resource.QualifiedResource;
import rcms.fm.resource.QualifiedResourceContainer;
import rcms.fm.resource.qualifiedresource.FunctionManager;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainer;
import rcms.fm.resource.qualifiedresource.XdaqExecutive;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;
import rcms.util.logger.RCMSLogger;
import rcms.utilities.runinfo.RunInfo;


/**
 * Example of Function Machine for controlling an Level 1 Function Manager.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 */
public class MyFunctionManager extends UserFunctionManager {

	/**
	 * <code>RCMSLogger</code>: RCMS log4j Logger
	 */
	static RCMSLogger logger = new RCMSLogger(MyFunctionManager.class);

	/**
	 * define some containers
	 */
	public XdaqApplicationContainer containerXdaqApplication = null;

	/**
	 * define specific application containers
	 */
	public XdaqApplicationContainer cEVM = null;

	/**
	 * <code>containerXdaqExecutive</code>: container of XdaqExecutive in the
	 * running Group.
	 */
	public XdaqApplicationContainer containerXdaqExecutive = null;

	/**
	 * <code>containerFunctionManager</code>: container of FunctionManagers
	 * in the running Group.
	 */
	public QualifiedResourceContainer containerFunctionManager = null;

	/**
	 * <code>containerJobControl</code>: container of JobControl in the
	 * running Group.
	 */
	public QualifiedResourceContainer containerJobControl = null;

	/**
	 * <code>calcState</code>: Calculated State.
	 */
	public State calcState = null;

        // RunInfo stuff:
	public RunInfo _myRunInfo = null;
	public RunInfo _myRunInfoDESTROY = null;

	public XdaqApplicationContainer xdaqSupervisor = null;

	// utlitity functions handle
	public MyUtil _myUtil;

	/**
	 * Instantiates an MyFunctionManager.
	 */
	public MyFunctionManager() {
		//
		// Any State Machine Implementation must provide the framework
		// with some information about itself.
		//

		// make the parameters available
		addParameters();

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see rcms.statemachine.user.UserStateMachine#createAction()
	 */
	public void createAction(ParameterSet<CommandParameter> pars) throws UserActionException {
		//
		// This method is called by the framework when the Function Manager is
		// created.

		System.out.println("createAction called.");
		logger.debug("createAction called.");

		_myUtil.killOrphanedExecutives();

		System.out.println("createAction executed.");
		logger.debug("createAction executed.");

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see rcms.statemachine.user.UserStateMachine#destroyAction()
	 */
	public void destroyAction() throws UserActionException {
		//
		// This method is called by the framework when the Function Manager is
		// destroyed.
		//

		System.out.println("destroyAction called");
		logger.debug("destroyAction called");

		logger.debug("cscFM destroyAction");

		QualifiedGroup group = getQualifiedGroup();

		List<QualifiedResource> list;

		// destroy XDAQ executives
		list = group.seekQualifiedResourcesOfType(new XdaqExecutive());

		for (QualifiedResource r: list) {
			logger.debug("==== killing " + r.getURI());
			try {
				((XdaqExecutive)r).killMe();
			} catch (Exception e) {
				logger.error(
						"Could not destroy a XDAQ executive " + r.getURI(), e);
			}
		}

		// destroy function managers
		list = group.seekQualifiedResourcesOfType(new FunctionManager());

		for (QualifiedResource r: list) {
			logger.debug("==== killing " + r.getURI());

			FunctionManager fm = (FunctionManager)r;

			if (fm.isInitialized()) {
				try {
					fm.destroy();
				} catch (Exception e) {
					logger.error("Could not destroy a FM " + r.getURI(), e);
				}
			}
		}
		
		System.out.println("destroyAction executed");
		logger.debug("destroyAction executed");
	}

	/**
	 * add parameters to parameterSet. After this they are accessible.
	 */
	private void addParameters() {

		// add parameters to parameter Set so they are visible.
		parameterSet = MyParameters.LVL_ONE_PARAMETER_SET;

	}

	public void init() throws StateMachineDefinitionException,
			rcms.fm.fw.EventHandlerException {

		//instantiate utility
		_myUtil = new MyUtil(this);

		//
		// Set first of all the State Machine Definition
		//
		setStateMachineDefinition(new MyStateMachineDefinition());

		//
		// Add event handler
		//
		addEventHandler(new MyEventHandler());

		addEventHandler(new MySetParameterHandler());
		
		//
		// Add error handler
		//
		addEventHandler(new MyErrorHandler());

		// call renderers
	    _myUtil.renderMainGui();

	}
	/**
	 * Returns true if custom GUI is required, false otherwise
	 * 
	 * @return true, because MyFunctionManager class requires user code
	 */	
	public boolean hasCustomGUI() {
		return true;
	}

}
